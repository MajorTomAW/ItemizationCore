// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryErrorDefinitions.h"
#include "InventoryOpData.h"
#include "InventoryOpHandle.h"
#include "InventoryResult.h"

namespace UE::Itemization
{
	class FInventoryError;
	template <typename OpType> class TInventoryAsyncOp;
	
	namespace Private
	{
		template <typename Outer, typename OpType, typename LastResultType>
		class TInventoryAsyncOpBase
		{
		public:
			TInventoryAsyncOpBase(LastResultType& InLastResult)
				: LastResult(InLastResult)
			{
			}

			template <typename CallableType>
			auto Then(CallableType&& Callable);
		
		protected:
			LastResultType& LastResult;
		};

		template <typename Outer, typename OpType>
		class TInventoryAsyncOpBase<Outer, OpType, void>
		{
		public:
			TInventoryAsyncOpBase() {}

			template <typename CallableType>
			auto Then(CallableType&& Callable);
		};

		// The things underneath are really concerning about my mental health,
		// but I guess I need them to make async stuff work
		
		template <typename... ParamTypes>
		struct TInventoryAsyncOpCallableTraitsHelper2
		{
		};
		template <typename TResultType, typename OpType>
		struct TInventoryAsyncOpCallableTraitsHelper2<TResultType, OpType&>
		{
			using ParamType = void;
			using ResultType = TResultType;
			static constexpr bool bAsyncResult = false;
			static constexpr bool bRequiresPromise = false;
		};
		template <typename TResultType, typename OpType, typename TParamType>
		struct TInventoryAsyncOpCallableTraitsHelper2<TResultType, OpType&, TParamType>
		{
			using ParamType = TParamType;
			using ResultType = TResultType;
			static constexpr bool bAsyncResult = false;
			static constexpr bool bRequiresPromise = false;
		};
		template <typename TResultType, typename OpType>
		struct TInventoryAsyncOpCallableTraitsHelper2<TFuture<TResultType>, OpType&>
		{
			using ParamType = void;
			using ResultType = TResultType;
			static constexpr bool bAsyncResult = true;
			static constexpr bool bRequiresPromise = false;
		};
		template <typename TResultType, typename OpType, typename TParamType>
		struct TInventoryAsyncOpCallableTraitsHelper2<TFuture<TResultType>, OpType&, TParamType>
		{
			using ParamType = TParamType;
			using ResultType = TResultType;
			static constexpr bool bAsyncResult = true;
			static constexpr bool bRequiresPromise = false;
		};
		template <typename TResultType, typename OpType>
		struct TInventoryAsyncOpCallableTraitsHelper2<void, OpType&, TPromise<TResultType>&&>
		{
			using ParamType = void;
			using ResultType = TResultType;
			static constexpr bool bAsyncResult = true;
			static constexpr bool bRequiresPromise = true;
		};
		template <typename TResultType, typename OpType, typename TParamType>
		struct TInventoryAsyncOpCallableTraitsHelper2<void, OpType&, TParamType, TPromise<TResultType>&&>
		{
			using ParamType = TParamType;
			using ResultType = TResultType;
			static constexpr bool bAsyncResult = true;
			static constexpr bool bRequiresPromise = true;
		};

		template <typename CallableType>
		struct TInventoryAsyncOpCallableTraitsHelper
		{
		};
		// The same thing as above, but with return and param types
		template <typename ReturnType, typename... ParamTypes>
		struct TInventoryAsyncOpCallableTraitsHelper<ReturnType(ParamTypes...)>
			: public TInventoryAsyncOpCallableTraitsHelper2<ReturnType, ParamTypes...>
		{
		};
		// Again the same thing, but with object type
		template <typename ReturnType, typename ObjectType, typename... ParamTypes>
		struct TInventoryAsyncOpCallableTraitsHelper<ReturnType(ObjectType::*)(ParamTypes...)>
			: public TInventoryAsyncOpCallableTraitsHelper2<ReturnType, ParamTypes...>
		{
		};
		// Same as above, but with const object type
		template <typename ReturnType, typename ObjectType, typename... ParamTypes>
		struct TInventoryAsyncOpCallableTraitsHelper<ReturnType(ObjectType::*)(ParamTypes...) const>
            : public TInventoryAsyncOpCallableTraitsHelper2<ReturnType, ParamTypes...>
        {
        };

		template <typename CallableType, typename = void>
		struct TInventoryAsyncOpCallableTraits
		{
		};
		// The same thing as above, just for function pointers
		template <typename CallableFunc>
		struct TInventoryAsyncOpCallableTraits<CallableFunc, std::enable_if_t<std::is_function_v<std::remove_pointer_t<CallableFunc>>, void>>
			: public TInventoryAsyncOpCallableTraitsHelper<CallableFunc>
		{
		};
		// Again the same thing, but for lambdas, TFunction, functor objects or basically anything with operator()
		template <typename CallableObj>
		struct TInventoryAsyncOpCallableTraits<CallableObj, std::enable_if_t<!std::is_function_v<std::remove_pointer_t<CallableObj>>, void>>
			: public TInventoryAsyncOpCallableTraitsHelper<decltype(&std::remove_reference_t<CallableObj>::operator())>
		{
		};

		/** Single step interface for async operations. */
		class IStep
		{
		public:
			virtual ~IStep() {}
			virtual void Execute() = 0;
		};

		template <typename ResultType>
		class TStep : public IStep
		{
		public:
			virtual ~TStep() override
			{
				// Welp, I think I have to destruct the result here by myself.
				if (bResultSet)
				{
					DestructItem(Result.GetTypedPtr());
				}
			}

			ResultType& GetResult_Ref()
			{
				return *Result.GetTypedPtr();
			}

			template <typename OpType, typename LastResultType, typename CallableType>
			void SetExecFunc(TInventoryAsyncOp<OpType>& InOp, LastResultType&& InLastResult, CallableType&& InCallable)
			{
				ExecFunc = [this,
					WeakOp = TWeakPtr<TInventoryAsyncOp<OpType>>(InOp.AsShared()),
					&LastResult = InLastResult,
					Callable = MoveTemp(InCallable)] () mutable
				{
					TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr = WeakOp.Pin();
					if (!OpPtr)
					{
						return;
					}

					if constexpr (TInventoryAsyncOpCallableTraits<CallableType>::bRequiresPromise)
					{
						TPromise<ResultType> Promise;

						// Continue the promise first, before calling the callable itself
						// Just to make sure that the promise is set before the callable is executed
						Promise.GetFuture()
						.Next([this, WeakOp] (const ResultType& Value)
						{
							if (TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr2 = WeakOp.Pin())
							{
								EmplaceResult(Value);
								OpPtr2->ExecuteNextStep();
							}
						});
					}
					else if constexpr (TInventoryAsyncOpCallableTraits<CallableType>::bAsyncResult)
					{
						Callable(*OpPtr, MoveTempIfPossible(LastResult))
						.Next([this, WeakOp] (const ResultType& Value)
						{
							if (TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr2 = WeakOp.Pin())
                            {
                                EmplaceResult(Value);
                                OpPtr2->ExecuteNextStep();
                            }
						});
					}
					else
					{
						EmplaceResult(Callable(*OpPtr, MoveTempIfPossible(LastResult)));
						OpPtr->ExecuteNextStep();
					}
				};
			}

			template <typename OpType, typename CallableType>
			void SetExecFunc(TInventoryAsyncOp<OpType>& InOp, CallableType&& InCallable)
			{
				ExecFunc = [this,
					WeakOp = TWeakPtr<TInventoryAsyncOp<OpType>>(InOp.AsShared()),
					Callable = MoveTemp(InCallable)] () mutable
				{
					TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr = WeakOp.Pin();
					if (!OpPtr)
					{
						return;
					}

					if constexpr (TInventoryAsyncOpCallableTraits<CallableType>::bRequiresPromise)
					{
						TPromise<ResultType> Promise;

						// Continue the promise first, before calling the callable itself
						// Just to make sure that the promise is set before the callable is executed
						Promise.GetFuture()
                        .Next([this, WeakOp] (const ResultType& Value)
                        {
                            if (TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr2 = WeakOp.Pin())
                            {
                                EmplaceResult(Value);
                                OpPtr2->ExecuteNextStep();
                            }
                        });
                    }
                    else if constexpr (TInventoryAsyncOpCallableTraits<CallableType>::bAsyncResult)
                    {
                        Callable(*OpPtr)
                        .Next([this, WeakOp] (const ResultType& Value)
                        {
                            if (TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr2 = WeakOp.Pin())
                            {
                                EmplaceResult(Value);
                                OpPtr2->ExecuteNextStep();
                            }
                        });
                    }
                    else
                    {
                        EmplaceResult(Callable(*OpPtr));
                        OpPtr->ExecuteNextStep();
					}
				};
			}

			virtual void Execute() override
			{
				check(ExecFunc);
				ExecFunc();
			}

		private:
			template <typename... ArgTypes>
			void EmplaceResult(ArgTypes&&... Args)
			{
				check(!bResultSet);
				new(Result.GetTypedPtr()) ResultType(Forward<ArgTypes>(Args)...);
				bResultSet = true;
			}
			
			TUniqueFunction<void()> ExecFunc;
			TTypeCompatibleBytes<ResultType> Result;
			bool bResultSet = false;
		};

		template <>
		class TStep<void> : public IStep
		{
			
		};
	}

	template <typename OpType, typename T>
	class TInventoryChainableAsyncOp
		: public Private::TInventoryAsyncOpBase<TInventoryChainableAsyncOp<OpType, T>, OpType, T>
	{
	public:
		using Super = Private::TInventoryAsyncOpBase<TInventoryChainableAsyncOp<OpType, T>, OpType, T>;

		TInventoryChainableAsyncOp(TInventoryAsyncOp<OpType>& InOwningOp, std::enable_if_t<!std::is_same_v<T, void>, T>& InLastResult)
			: Super(InLastResult)
			, OwningOp(InOwningOp)
		{
		}

		TInventoryChainableAsyncOp(TInventoryChainableAsyncOp&& Other)
			: OwningOp(Other.OwningOp)
		{
		}

		TInventoryChainableAsyncOp& operator=(TInventoryChainableAsyncOp&& Other)
		{
			check(&OwningOp == &Other.OwningOp);
			Super::operator=(MoveTemp(Other));
			return *this;
		}

		template <typename QueueType>
		void Enqueue(QueueType& Queue)
		{
			static_assert(std::is_same_v<T, void>,
				"Continuation result discarded. Continuation prior to calling Enqueue must have a void or TFuture<void> return type.");
			OwningOp.Enqueue(Queue);
		}

		TInventoryAsyncOp<OpType>& GetOwningOp()
		{
			return OwningOp;
		}

	protected:
		TInventoryAsyncOp<OpType>& OwningOp;
	};
	template <typename OpType>
	class TInventoryChainableAsyncOp<OpType, void>
		: public Private::TInventoryAsyncOpBase<TInventoryChainableAsyncOp<OpType, void>, OpType, void>
	{
	public:
		using Super = Private::TInventoryAsyncOpBase<TInventoryChainableAsyncOp<OpType, void>, OpType, void>;

		TInventoryChainableAsyncOp(TInventoryAsyncOp<OpType>& InOwningOp)
			: Super()
			, OwningOp(InOwningOp)
		{
		}
		TInventoryChainableAsyncOp(TInventoryChainableAsyncOp&& Other)
			: OwningOp(Other.OwningOp)
		{
		}

		TInventoryChainableAsyncOp& operator=(TInventoryChainableAsyncOp&& Other)
		{
			check(&OwningOp == &Other.OwningOp);
			Super::operator=(MoveTemp(Other));
			return *this;
		}

		template <typename QueueType>
		void Enqueue(QueueType& Queue)
		{
			OwningOp.Enqueue(Queue);
		}

		TInventoryAsyncOp<OpType>& GetOwningOp()
		{
			return OwningOp;
		}

	protected:
		TInventoryAsyncOp<OpType>& OwningOp;
	};

	class FInventoryAsyncOp
	{
	public:
		virtual ~FInventoryAsyncOp() {}
		virtual void SetError(FInventoryError&& Error) = 0;

		// Some data cache we can access across async op steps
		Private::FInventoryOpData Data;
	};

	template <typename OpType>
	class TInventoryAsyncOp
		: public Private::TInventoryAsyncOpBase<TInventoryAsyncOp<OpType>, OpType, void>
		, public FInventoryAsyncOp
		, public TSharedFromThis<TInventoryAsyncOp<OpType>>
	{
	public:
		using ParamsType = typename OpType::Params;
		using ResultType = typename OpType::Result;

		TInventoryAsyncOp(ParamsType&& Params)
			: SharedState(MakeShared<FInventoryOpSharedState>(MoveTemp(Params)))
			, OpStartTimeAbsSec(FPlatformTime::Seconds())
		{
		}

		virtual ~TInventoryAsyncOp() override {}

		bool IsReady() const
		{
			return SharedState->State != EInventoryOpState::Invalid;
		}
		
		bool IsComplete() const
		{
			return SharedState->State <= EInventoryOpState::Completed;
		}

		EInventoryOpState GetState() const
		{
			return SharedState->State;
		}

		void Cancel(const FInventoryError& Reason)
		{
			SetResultAndSate(TInventoryTransactionResult<OpType>(Reason), EInventoryOpState::Cancelled);
		}

		const ParamsType& GetParams() const
		{
			return SharedState->Params;
		}

		TInventoryAsyncOp<OpType>& GetOwningOp()
		{
			return *this;
		}

		TInventoryOpHandle<OpType> GetHandle()
		{
			return TInventoryOpHandle<OpType>(CreateSharedState());
		}

		virtual void SetError(FInventoryError&& Error) override
		{
			SetResultAndSate(TInventoryTransactionResult<OpType>(MoveTemp(Error)), EInventoryOpState::Completed);
		}

		void SetResult(ResultType&& InResult)
		{
			SetResultAndSate(TInventoryTransactionResult<OpType>(MoveTemp(InResult)), EInventoryOpState::Completed);
		}

		template <typename QueueType>
		void Enqueue(QueueType& Queue)
		{
			check(SharedState->State < EInventoryOpState::Queued)
			SharedState->State = EInventoryOpState::Queued;
			Queue.Enqueue(*this);
		}

		void Start()
		{
			SharedState->State = EInventoryOpState::Running;
			//@TODO: OnStartEvent delegate
			ExecuteNextStep();
		}

		void ExecuteNextStep()
		{
			if (!IsComplete())
			{
				const int StepIdx = NextStep;
				++NextStep;

				if (StepIdx < Steps.Num())
				{
					Exec([this,
						StepIdx,
						WeakThis = TWeakPtr<TInventoryAsyncOp<OpType>>(this->AsShared())] ()
					{
						TSharedPtr<TInventoryAsyncOp<OpType>> ThisPtr = WeakThis.Pin();
						if (ThisPtr)
						{
							Steps[StepIdx]->Execute();
						}
					});
				}
			}
		}

		void AddStep(TUniquePtr<Private::IStep>&& Step)
		{
			Steps.Add(MoveTemp(Step));
		}

		template <typename CallableType>
		void Exec(CallableType&& Callable)
		{
			if (IsInGameThread())
			{
				Callable();
			}
			else
			{
				Async(EAsyncExecution::TaskGraphMainThread, MoveTemp(Callable));
			}
		}

	protected:
		void SetResultAndSate(TInventoryTransactionResult<OpType>&& Result, EInventoryOpState State)
		{
			SharedState->Result = MoveTemp(Result);
			SharedState->State = State;

			TriggerOnComplete(SharedState->Result);
		}

		void TriggerOnComplete(const TInventoryTransactionResult<OpType>& Result)
		{
			TArray<TSharedRef<FInventoryOpSharedHandleState>> SharedHandleStatesCopy(SharedHandleStates);
			for (TSharedRef<FInventoryOpSharedHandleState>& HandleState : SharedHandleStatesCopy)
			{
				HandleState->TriggerOnComplete(Result);
			}

			//@TODO: OnCompleteEvent delegate

			double DurationInS = FPlatformTime::Seconds() - OpStartTimeAbsSec;

			//@TODO: OnInventoryAsyncOpCompleted delegate
		}
	
		class FInventoryOpSharedState
		{
		public:
			FInventoryOpSharedState(ParamsType&& InParams)
				: Params(MoveTemp(InParams))
			{
			}
		
			ParamsType Params;
			TInventoryTransactionResult<OpType> Result;
			EInventoryOpState State = EInventoryOpState::Invalid;

			bool IsComplete() const
			{
				return State >= EInventoryOpState::Completed;
			}
		};

		class FInventoryOpSharedHandleState
			: public Private::IInventoryOpSharedState<OpType>
			, public TSharedFromThis<FInventoryOpSharedHandleState>
		{
		public:
			FInventoryOpSharedHandleState(const TSharedRef<TInventoryAsyncOp<OpType>>& InOp)
				: SharedState(InOp->SharedState)
				, Op(InOp)
			{
			}
		
			virtual EInventoryOpState GetState() const override
			{
				return SharedState->State;
			}

			virtual void Cancel(const FInventoryError& Reason)
			{
				TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr = Op.Pin();
				if (OpPtr.IsValid())
				{
					bCancelled = true;

					// So when we cancel something, we need to set the error code of the outer op
					if (Reason.GetErrorCode() == Errors::ErrorCode::Common::Cancelled)
					{
						TriggerOnComplete(TInventoryTransactionResult<OpType>(Reason));
					}
					else
					{
						TriggerOnComplete(TInventoryTransactionResult<OpType>(Errors::Cancelled(Reason)));
					}
				}
			}

			void TriggerOnComplete(const TInventoryTransactionResult<OpType>& Result)
			{
				//@TODO: OnCompleteFn delegate execute and unbind and detach
			}
		
		private:
			void Detach()
			{
				TSharedPtr<TInventoryAsyncOp<OpType>> OpPtr = Op.Pin();
				Op.Reset();
				if (OpPtr.IsValid())
				{
					OpPtr->Detach(this->AsShared());
				}
			}
			
			bool bCancelled = false;
			TSharedRef<FInventoryOpSharedState> SharedState;
			TWeakPtr<TInventoryAsyncOp<OpType>> Op;
		};

		void Detach(const TSharedRef<FInventoryOpSharedHandleState>& SharedHandleState)
		{
			SharedHandleStates.Remove(SharedHandleState);
		}

		TSharedRef<Private::IInventoryOpSharedState<OpType>> CreateSharedState()
		{
			TSharedRef<FInventoryOpSharedHandleState> NewHandleState = MakeShared<FInventoryOpSharedHandleState>(this->AsShared());
			SharedHandleStates.Add(NewHandleState);
			return StaticCastSharedRef<Private::IInventoryOpSharedState<OpType>>(NewHandleState);
		}

		TSharedRef<FInventoryOpSharedState> SharedState;
		TArray<TSharedRef<FInventoryOpSharedHandleState>> SharedHandleStates;
		TArray<TUniquePtr<Private::IStep>> Steps;
		int NextStep = 0;
		double OpStartTimeAbsSec;
	};

	namespace Private
	{
		template <typename Outer, typename OpType, typename LastResultType>
		template <typename CallableType>
		auto TInventoryAsyncOpBase<Outer, OpType, LastResultType>::Then(CallableType&& Callable)
		{
			using ResultType = typename TInventoryAsyncOpCallableTraits<CallableType>::ResultType;
			TInventoryAsyncOp<OpType>& Op = static_cast<Outer*>(this)->GetOwningOp();

			TStep<ResultType>* Step = new TStep<ResultType>();
			TUniquePtr<IStep> StepPtr(Step);

			Step->SetExecFunc(Op, MoveTemp(Callable));
			Op.AddStep(MoveTemp(StepPtr));

			if constexpr (std::is_same_v<OpType, void>)
			{
				return TInventoryChainableAsyncOp<OpType, ResultType>(Op);
			}
			else
			{
				return TInventoryChainableAsyncOp<OpType, ResultType>(Op, Step->GetResult_Ref());
			}	
		}

		template <typename Outer, typename OpType>
		template <typename CallableType>
		auto TInventoryAsyncOpBase<Outer, OpType, void>::Then(CallableType&& Callable)
		{
			using ResultType = typename TInventoryAsyncOpCallableTraits<CallableType>::ResultType;
			TInventoryAsyncOp<OpType>& Op = static_cast<Outer*>(this)->GetOwningOp();

			TStep<ResultType>* Step = new TStep<ResultType>();
			TUniquePtr<IStep> StepPtr(Step);
			
			Step->SetExecFunc(Op, MoveTemp(Callable));
			Op.AddStep(MoveTemp(StepPtr));

			if constexpr (std::is_same_v<OpType, void>)
			{
				return TInventoryChainableAsyncOp<OpType, ResultType>(Op);
			}
			else
			{
				return TInventoryChainableAsyncOp<OpType, ResultType>(Op, Step->GetResult_Ref());
			}
		}
	}


	template <typename OpType>
	using TInventoryOpRef = TSharedRef<TInventoryAsyncOp<OpType>>;
	template <typename OpType>
	using TInventoryOpPtr = TSharedPtr<TInventoryAsyncOp<OpType>>;
}