// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a "No Compile Test" suite.
// http://dev.chromium.org/developers/testing/no-compile-tests

#define FORCE_UNRETAINED_COMPLETENESS_CHECKS_FOR_TESTS 1

#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/functional/disallow_unretained.h"
#include "base/memory/ref_counted.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/raw_ref.h"
#include "base/test/bind.h"

namespace base {

// Do not put everything inside an anonymous namespace.  If you do, many of the
// helper function declarations will generate unused definition warnings.

static const int kParentValue = 1;
static const int kChildValue = 2;

class NoRef {
 public:
  void VoidMethod0() {}
  void VoidConstMethod0() const {}
  int IntMethod0() { return 1; }
};

class HasRef : public NoRef, public base::RefCounted<HasRef> {
};

class Parent {
 public:
  void AddRef() const {}
  void Release() const {}
  virtual void VirtualSet() { value = kParentValue; }
  void NonVirtualSet() { value = kParentValue; }
  int value;
};

class Child : public Parent {
 public:
  virtual void VirtualSet() { value = kChildValue; }
  void NonVirtualSet() { value = kChildValue; }
};

class NoRefParent {
 public:
  virtual void VirtualSet() { value = kParentValue; }
  void NonVirtualSet() { value = kParentValue; }
  int value;
};

class NoRefChild : public NoRefParent {
  virtual void VirtualSet() { value = kChildValue; }
  void NonVirtualSet() { value = kChildValue; }
};

template <typename T>
T PolymorphicIdentity(T t) {
  return t;
}

int UnwrapParentRef(Parent& p) {
  return p.value;
}

template <typename T>
void VoidPolymorphic1(T t) {
}

void TakesMoveOnly(std::unique_ptr<int>) {
}

void TakesIntRef(int& ref) {}

struct NonEmptyFunctor {
  int x;
  void operator()() const {}
};

class Incomplete;
void PassIncompleteByPtr(Incomplete*) {}

class Dangerous {
 public:
  void Method() {}

  DISALLOW_UNRETAINED();
};

void PassDangerousByPtr(Dangerous*) {}
void PassDangerousByConstRef(const Dangerous&) {}
void PassDangerousByMutableRef(Dangerous&) {}


#if defined(NCTEST_METHOD_ON_CONST_OBJECT)  // [r"Type mismatch between bound argument and bound functor's parameter\."]

// Method bound to const-object.
//
// Only const methods should be allowed to work with const objects.
void WontCompile() {
  HasRef has_ref;
  const HasRef* const_has_ref_ptr_ = &has_ref;
  RepeatingCallback<void()> method_to_const_cb =
      BindRepeating(&HasRef::VoidMethod0, const_has_ref_ptr_);
  method_to_const_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_NEEDS_REFCOUNTED_OBJECT)  // [r"Receivers may not be raw pointers. If using a raw pointer here is safe and has no lifetime concerns, use base::Unretained\(\) and document why it's safe."]


// Method bound to non-refcounted object.
//
// We require refcounts unless you have Unretained().
void WontCompile() {
  NoRef no_ref;
  RepeatingCallback<void()> no_ref_cb =
      BindRepeating(&NoRef::VoidMethod0, &no_ref);
  no_ref_cb.Run();
}

#elif defined(NCTEST_CONST_METHOD_BIND_NEEDS_REFCOUNTED_OBJECT)  // [r"Receivers may not be raw pointers. If using a raw pointer here is safe and has no lifetime concerns, use base::Unretained\(\) and document why it's safe."]

// Const Method bound to non-refcounted object.
//
// We require refcounts unless you have Unretained().
void WontCompile() {
  NoRef no_ref;
  RepeatingCallback<void()> no_ref_const_cb =
      BindRepeating(&NoRef::VoidConstMethod0, &no_ref);
  no_ref_const_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_RAW_PTR_RECEIVER_NEEDS_REFCOUNTED_OBJECT)  // [r"Receivers may not be raw pointers. If using a raw pointer here is safe and has no lifetime concerns, use base::Unretained\(\) and document why it's safe."]


// Method bound to non-refcounted object.
//
// We require refcounts unless you have Unretained().
void WontCompile() {
  NoRef no_ref;
  raw_ptr<NoRef> rawptr(&no_ref);
  RepeatingCallback<void()> no_ref_cb =
      BindRepeating(&NoRef::VoidMethod0, rawptr);
  no_ref_cb.Run();
}

#elif defined(NCTEST_CONST_METHOD_BIND_RAW_PTR_RECEIVER_NEEDS_REFCOUNTED_OBJECT)  // [r"Receivers may not be raw pointers. If using a raw pointer here is safe and has no lifetime concerns, use base::Unretained\(\) and document why it's safe."]

// Const Method bound to non-refcounted object.
//
// We require refcounts unless you have Unretained().
void WontCompile() {
  NoRef no_ref;
  raw_ptr<NoRef> rawptr(&no_ref);
  RepeatingCallback<void()> no_ref_const_cb =
      BindRepeating(&NoRef::VoidConstMethod0, rawptr);
  no_ref_const_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_REF_WRAPPER_RECEIVER_NON_REFCOUNTED_OBJECT)  // [r"Cannot convert `this` argument to address\. Method calls must be bound using a pointer-like `this` argument\."]

// Method bound to non-refcounted object. It fails to compile with
// std::reference_wrapper.
void WontCompile() {
  NoRef no_ref;
  RepeatingCallback<void()> no_ref_cb =
      BindRepeating(&NoRef::VoidMethod0, std::cref(no_ref));
  no_ref_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_NATIVE_REF_RECEIVER_NON_REFCOUNTED_OBJECT)  // [r"Cannot convert `this` argument to address\. Method calls must be bound using a pointer-like `this` argument\."]

// Method bound to non-refcounted object. It fails to compile with
// a native reference.
void WontCompile() {
  NoRef no_ref;
  RepeatingCallback<void()> no_ref_cb =
      BindRepeating(&NoRef::VoidMethod0, no_ref);
  no_ref_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_RAW_REF_RECEIVER_NON_REFCOUNTED_OBJECT)  // [r"Receivers may not be raw_ref<T>\."]

// Method bound to non-refcounted object. It fails to compile with
// a raw_ref.
void WontCompile() {
  NoRef no_ref;
  raw_ref<NoRef> rawref(no_ref);
  RepeatingCallback<void()> no_ref_cb =
      BindRepeating(&NoRef::VoidMethod0, rawref);
  no_ref_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_REF_WRAPPER_RECEIVER_REFCOUNTED_OBJECT)  // [r"Cannot convert `this` argument to address\. Method calls must be bound using a pointer-like `this` argument\."]

// Method bound to non-refcounted object. It fails to compile with
// std::reference_wrapper.
void WontCompile() {
  HasRef has_ref;
  RepeatingCallback<void()> has_ref_cb =
      BindRepeating(&HasRef::VoidMethod0, std::cref(has_ref));
  has_ref_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_NATIVE_REF_RECEIVER_REFCOUNTED_OBJECT)  // [r"Cannot convert `this` argument to address\. Method calls must be bound using a pointer-like `this` argument\."]

// Method bound to non-refcounted object. It fails to compile with
// a native reference.
void WontCompile() {
  HasRef has_ref;
  RepeatingCallback<void()> has_ref_cb =
      BindRepeating(&HasRef::VoidMethod0, has_ref);
  has_ref_cb.Run();
}

#elif defined(NCTEST_METHOD_BIND_RAW_REF_RECEIVER_REFCOUNTED_OBJECT)  // [r"Receivers may not be raw_ref<T>\."]

// Method bound to non-refcounted object. It fails to compile with
// a raw_ref.
void WontCompile() {
  HasRef has_ref;
  raw_ref<HasRef> rawref(has_ref);
  RepeatingCallback<void()> has_ref_cb =
      BindRepeating(&HasRef::VoidMethod0, rawref);
  has_ref_cb.Run();
}

#elif defined(NCTEST_CONST_POINTER)  // [r"Type mismatch between bound argument and bound functor's parameter\."]
// Const argument used with non-const pointer parameter of same type.
//
// This is just a const-correctness check.
void WontCompile() {
  const NoRef* const_no_ref_ptr;
  RepeatingCallback<NoRef*()> pointer_same_cb =
      BindRepeating(&PolymorphicIdentity<NoRef*>, const_no_ref_ptr);
  pointer_same_cb.Run();
}

#elif defined(NCTEST_CONST_POINTER_SUBTYPE)  // [r"Type mismatch between bound argument and bound functor's parameter\."]

// Const argument used with non-const pointer parameter of super type.
//
// This is just a const-correctness check.
void WontCompile() {
  const NoRefChild* const_child_ptr;
  RepeatingCallback<NoRefParent*()> pointer_super_cb =
    BindRepeating(&PolymorphicIdentity<NoRefParent*>, const_child_ptr);
  pointer_super_cb.Run();
}

#elif defined(DISABLED_NCTEST_DISALLOW_NON_CONST_REF_PARAM)  // [r"no member named 'AddRef' in 'base::NoRef'"]
// TODO(dcheng): I think there's a type safety promotion issue here where we can
// pass a const ref to a non const-ref function, or vice versa accidentally. Or
// we make a copy accidentally. Check.

// Functions with reference parameters, unsupported.
//
// First, non-const reference parameters are disallowed by the Google
// style guide. Second, since we are doing argument forwarding it becomes
// very tricky to avoid copies, maintain const correctness, and not
// accidentally have the function be modifying a temporary, or a copy.
void WontCompile() {
  Parent p;
  RepeatingCallback<int(Parent&)> ref_arg_cb = BindRepeating(&UnwrapParentRef);
  ref_arg_cb.Run(p);
}

#elif defined(NCTEST_BIND_ONCE_WITH_NON_CONST_REF_PARAM)  // [r"Bound argument for non-const reference parameter must be wrapped in std::ref\(\) or base::OwnedRef\(\)."]

// Binding functions with reference parameters requires `std::ref()` or
// 'base::OwnedRef()`.
void WontCompile() {
  int v = 1;
  auto cb = BindOnce(&TakesIntRef, v);
}

#elif defined(NCTEST_BIND_REPEATING_WITH_NON_CONST_REF_PARAM)  // [r"Bound argument for non-const reference parameter must be wrapped in std::ref\(\) or base::OwnedRef\(\)."]

// Binding functions with reference parameters requires `std::ref()` or
// 'base::OwnedRef()`.
void WontCompile() {
  int v = 1;
  auto cb = BindRepeating(&TakesIntRef, v);
}

#elif defined(NCTEST_NON_CONST_REF_PARAM_WRONG_TYPE)  // [r"Type mismatch between bound argument and bound functor's parameter."]

// If the argument and parameter types mismatch then the compile error should be
// the generic type mismatch error.
void WontCompile() {
  float f = 1.0f;
  auto cb = BindOnce(&TakesIntRef, f);
}

#elif defined(NCTEST_NON_CONST_REF_PARAM_WRONG_TYPE_AND_WRAPPED)  // [r"Type mismatch between bound argument and bound functor's parameter."]

// If the argument and parameter types mismatch then the compile error should be
// the generic type mismatch error even if the argument is wrapped in
// base::OwnedRef().
void WontCompile() {
  float f = 1.0f;
  auto cb = BindOnce(&TakesIntRef, base::OwnedRef(f));
}

#elif defined(NCTEST_NO_IMPLICIT_ARRAY_PTR_CONVERSION)  // [r"First bound argument to a method cannot be an array."]

// A method should not be bindable with an array of objects.
//
// This is likely not wanted behavior. We specifically check for it though
// because it is possible, depending on how you implement prebinding, to
// implicitly convert an array type to a pointer type.
void WontCompile() {
  HasRef p[10];
  RepeatingCallback<void()> method_bound_to_array_cb =
      BindRepeating(&HasRef::VoidMethod0, p);
  method_bound_to_array_cb.Run();
}

#elif defined(NCTEST_NO_RVALUE_RAW_REF_FOR_REFCOUNTED_TYPES)  // [r"A parameter is a refcounted type and needs scoped_refptr."]

// Refcounted types should not be bound as a raw pointer.
void WontCompile() {
  HasRef has_ref;
  raw_ref<HasRef> rr(has_ref);
  int a;
  raw_ref<int> rr_a(a);
  RepeatingCallback<void()> ref_count_as_raw_ptr_a =
      BindRepeating(&VoidPolymorphic1<raw_ref<int>>, rr_a);
  RepeatingCallback<void()> ref_count_as_raw_ptr =
      BindRepeating(&VoidPolymorphic1<raw_ref<HasRef>>, rr);
}

#elif defined(NCTEST_NO_RVALUE_RAW_PTR_FOR_REFCOUNTED_TYPES)  // [r"A parameter is a refcounted type and needs scoped_refptr."]

// Refcounted types should not be bound as a raw pointer.
void WontCompile() {
  HasRef for_raw_ptr;
  int a;
  RepeatingCallback<void()> ref_count_as_raw_ptr_a =
      BindRepeating(&VoidPolymorphic1<int*>, &a);
  RepeatingCallback<void()> ref_count_as_raw_ptr =
      BindRepeating(&VoidPolymorphic1<HasRef*>, &for_raw_ptr);
}

#elif defined(NCTEST_NO_LVALUE_RAW_PTR_FOR_REFCOUNTED_TYPES)  // [r"A parameter is a refcounted type and needs scoped_refptr."]

// Refcounted types should not be bound as a raw pointer.
void WontCompile() {
  HasRef* for_raw_ptr = nullptr;
  RepeatingCallback<void()> ref_count_as_raw_ptr =
      BindRepeating(&VoidPolymorphic1<HasRef*>, for_raw_ptr);
}

#elif defined(NCTEST_NO_RVALUE_CONST_RAW_PTR_FOR_REFCOUNTED_TYPES)  // [r"A parameter is a refcounted type and needs scoped_refptr."]

// Refcounted types should not be bound as a raw pointer.
void WontCompile() {
  const HasRef for_raw_ptr;
  RepeatingCallback<void()> ref_count_as_raw_ptr =
      BindRepeating(&VoidPolymorphic1<const HasRef*>, &for_raw_ptr);
}

#elif defined(NCTEST_NO_LVALUE_CONST_RAW_PTR_FOR_REFCOUNTED_TYPES)  // [r"A parameter is a refcounted type and needs scoped_refptr."]

// Refcounted types should not be bound as a raw pointer.
void WontCompile() {
  const HasRef* for_raw_ptr = nullptr;
  RepeatingCallback<void()> ref_count_as_raw_ptr =
      BindRepeating(&VoidPolymorphic1<const HasRef*>, for_raw_ptr);
}

#elif defined(NCTEST_WEAKPTR_BIND_MUST_RETURN_VOID)  // [r"WeakPtrs can only bind to methods without return values."]

// WeakPtrs cannot be bound to methods with return types.
void WontCompile() {
  NoRef no_ref;
  WeakPtrFactory<NoRef> weak_factory(&no_ref);
  RepeatingCallback<int()> weak_ptr_with_non_void_return_type =
      BindRepeating(&NoRef::IntMethod0, weak_factory.GetWeakPtr());
  weak_ptr_with_non_void_return_type.Run();
}

#elif defined(NCTEST_DISALLOW_ASSIGN_DIFFERENT_TYPES)  // [r"no viable conversion from 'RepeatingCallback<UnboundRunType>' to 'RepeatingCallback<void \(\)>'"]

// Bind result cannot be assigned to Callbacks with a mismatching type.
void WontCompile() {
  RepeatingClosure callback_mismatches_bind_type =
      BindRepeating(&VoidPolymorphic1<int>);
}

#elif defined(NCTEST_DISALLOW_CAPTURING_LAMBDA)  // [r"Capturing lambdas and stateful lambdas are intentionally not supported\."]

void WontCompile() {
  int i = 0, j = 0;
  BindOnce([i,&j]() {j = i;});
}

#elif defined(NCTEST_DISALLOW_ONCECALLBACK_RUN_ON_LVALUE)  // [r"OnceCallback::Run\(\) may only be invoked on a non-const rvalue, i\.e\. std::move\(callback\).Run\(\)."]

void WontCompile() {
  OnceClosure cb = BindOnce([] {});
  cb.Run();
}

#elif defined(NCTEST_DISALLOW_ONCECALLBACK_RUN_ON_CONST_LVALUE)  // [r"OnceCallback::Run\(\) may only be invoked on a non-const rvalue, i\.e\. std::move\(callback\).Run\(\)."]

void WontCompile() {
  const OnceClosure cb = BindOnce([] {});
  cb.Run();
}

#elif defined(NCTEST_DISALLOW_ONCECALLBACK_RUN_ON_CONST_RVALUE)  // [r"OnceCallback::Run\(\) may only be invoked on a non-const rvalue, i\.e\. std::move\(callback\).Run\(\)."]

void WontCompile() {
  const OnceClosure cb = BindOnce([] {});
  std::move(cb).Run();
}

#elif defined(NCTEST_DISALLOW_BIND_ONCECALLBACK)  // [r"BindRepeating\(\) cannot bind OnceCallback. Use BindOnce\(\) with std::move\(\)\."]

void WontCompile() {
  BindRepeating(BindOnce([](int) {}), 42);
}

#elif defined(NCTEST_DISALLOW_BINDONCE_LVALUE_ONCECALLBACK)  // [r"BindOnce\(\) requires non-const rvalue for OnceCallback binding, i\.e\. base::BindOnce\(std::move\(callback\)\)\."]
void WontCompile() {
  auto cb = BindOnce([](int) {});
  BindOnce(cb, 42);
}

#elif defined(NCTEST_DISALLOW_BINDONCE_RVALUE_CONST_ONCECALLBACK)  // [r"BindOnce\(\) requires non-const rvalue for OnceCallback binding, i\.e\. base::BindOnce\(std::move\(callback\)\)\."]

void WontCompile() {
  const auto cb = BindOnce([](int) {});
  BindOnce(std::move(cb), 42);
}

#elif defined(NCTEST_BINDONCE_MOVEONLY_TYPE_BY_VALUE)  // [r"Attempting to bind a move-only type\. Use std::move\(\) to transfer ownership to the created callback\."]

void WontCompile() {
  std::unique_ptr<int> x;
  BindOnce(&TakesMoveOnly, x);
}

#elif defined(NCTEST_BIND_MOVEONLY_TYPE_BY_VALUE)  // [r"base::BindRepeating\(\) argument is a move-only type\. Use base::Passed\(\) instead of std::move\(\) to transfer ownership from the callback to the bound functor\."]

void WontCompile() {
  std::unique_ptr<int> x;
  BindRepeating(&TakesMoveOnly, x);
}

#elif defined(NCTEST_BIND_MOVEONLY_TYPE_WITH_STDMOVE)  // [r"base::BindRepeating\(\) argument is a move-only type\. Use base::Passed\(\) instead of std::move\(\) to transfer ownership from the callback to the bound functor\."]

void WontCompile() {
  std::unique_ptr<int> x;
  BindRepeating(&TakesMoveOnly, std::move(x));
}

#elif defined(NCTEST_BIND_NON_EMPTY_FUNCTOR)  // [r"Capturing lambdas and stateful lambdas are intentionally not supported\."]

void WontCompile() {
  BindRepeating(NonEmptyFunctor());
}

#elif defined(NCTEST_DISALLOW_BINDLAMBDAFORTESTING_LVALUE_MUTABLE_LAMBDA)  // [r"BindLambdaForTesting\(\) requires non-const rvalue for mutable lambda binding, i\.e\. base::BindLambdaForTesting\(std::move\(lambda\)\)\."]
void WontCompile() {
  int foo = 42;
  auto mutable_lambda = [&]() mutable {};
  BindLambdaForTesting(mutable_lambda);
}

#elif defined(NCTEST_DISALLOW_BINDLAMBDAFORTESTING_RVALUE_CONST_MUTABLE_LAMBDA)  // [r"BindLambdaForTesting\(\) requires non-const rvalue for mutable lambda binding, i\.e\. base::BindLambdaForTesting\(std::move\(lambda\)\)\."]

void WontCompile() {
  int foo = 42;
  const auto mutable_lambda = [&]() mutable {};
  BindLambdaForTesting(std::move(mutable_lambda));
}

#elif defined(NCTEST_BIND_UNCOPYABLE_AND_UNMOVABLE_TYPE)  // [r"Cannot capture argument: is the argument copyable or movable\?"]

void WontCompile() {
  struct UncopyableUnmovable {
    UncopyableUnmovable() = default;
    UncopyableUnmovable(const UncopyableUnmovable&) = delete;
    UncopyableUnmovable& operator=(const UncopyableUnmovable&) = delete;
  };

  UncopyableUnmovable u;
  BindOnce([] (const UncopyableUnmovable&) {}, u);
}

#elif defined(NCTEST_BIND_ONCE_WITH_PASSED)  // [r"Use std::move\(\) instead of base::Passed\(\) with base::BindOnce\(\)\."]

void WontCompile() {
  std::unique_ptr<int> x;
  BindOnce([] (std::unique_ptr<int>) {}, Passed(&x));
}

#elif defined(NCTEST_BIND_ONCE_WITH_ADDRESS_OF_OVERLOADED_FUNCTION)  // [r"reference to overloaded function could not be resolved; did you mean to call it\?"]

void F(int);
void F(float);

void WontCompile() {
  BindOnce(&F, 1, 2, 3);
}

#elif defined(NCTEST_BIND_REPEATING_WITH_ADDRESS_OF_OVERLOADED_FUNCTION)  // [r"reference to overloaded function could not be resolved; did you mean to call it\?"]

void F(int);
void F(float);

void WontCompile() {
  BindRepeating(&F, 1, 2, 3);
}

#elif defined(NCTEST_UNRETAINED_WITH_INCOMPLETE_TYPE)  // [r"T must be fully defined\."]

void WontCompile(Incomplete* incomplete) {
  BindOnce(&PassIncompleteByPtr, base::Unretained(incomplete));
}

#elif defined(NCTEST_RAW_POINTER_WITH_INCOMPLETE_TYPE)  // [r"T must be fully defined\."]

void WontCompile(Incomplete* incomplete) {
  BindOnce(&PassIncompleteByPtr, incomplete);
}

#elif defined(NCTEST_UNRETAINED_WITH_DISALLOWED_TYPE)  // [r"Callback cannot capture an unprotected C\+\+ pointer since this type is annotated with DISALLOW_UNRETAINED\(\)\."]

void WontCompile() {
  Dangerous dangerous;
  BindOnce(&Dangerous::Method, base::Unretained(&dangerous));
}

#elif defined(NCTEST_RAW_POINTER_WITH_DISALLOWED_TYPE)  // [r"Callback cannot capture an unprotected C\+\+ pointer since this type is annotated with DISALLOW_UNRETAINED\(\)\."]

void WontCompile() {
  Dangerous dangerous;
  BindOnce(&PassDangerousByPtr, &dangerous);
}

#elif defined(NCTEST_RAW_CONST_REFERENCE_WITH_DISALLOWED_TYPE)  // [r"Callback cannot capture an unprotected C\+\+ reference since this type is annotated with DISALLOW_UNRETAINED\(\)\."]

void WontCompile() {
  Dangerous dangerous;
  BindOnce(&PassDangerousByConstRef, std::cref(dangerous));
}

#elif defined(NCTEST_RAW_MUTABLE_REFERENCE_WITH_DISALLOWED_TYPE)  // [r"Callback cannot capture an unprotected C\+\+ reference since this type is annotated with DISALLOW_UNRETAINED\(\)\."]

void WontCompile() {
  Dangerous dangerous;
  BindOnce(&PassDangerousByMutableRef, std::ref(dangerous));
}

#elif defined(NCTEST_UNSAFE_DANLING_WITHOUT_RAW_PTR_RECEIVER) // [r"base::UnsafeDangling\(\) pointers must be received by functors with MayBeDangling<T> as parameter\."]

void PassIntPtr(int *ptr) {}

void WontCompile() {
  int val;
  BindOnce(&PassIntPtr, UnsafeDangling(&val));
}

#elif defined(NCTEST_UNSAFE_DANLING_WITH_DIFFERENT_PTR_TRAITS) // [r"MayBeDangling<T> parameter must receive the same RawPtrTraits as the one passed to the corresponding base::UnsafeDangling\(\) call\."]

void PassIntPtr(MayBeDangling<int> ptr) {}

void WontCompile() {
  int val;
  BindOnce(&PassIntPtr, UnsafeDangling<int, RawPtrTraits::kDummyForTest>(&val));
}

#elif defined(NCTEST_UNSAFE_DANLING_UNTRIAGED_WITH_RAW_PTR_RECEIVER) // [r"base::Bind\(\) target functor has a parameter of type raw_ptr<T>."]

void PassIntRawPtr(raw_ptr<int> ptr) {}

void WontCompile() {
  int val;
  BindOnce(&PassIntRawPtr, UnsafeDanglingUntriaged(&val));
}

#endif

}  // namespace base
