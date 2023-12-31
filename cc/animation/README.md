# cc/animation

[TOC]

## Overview

cc/animation provides animation support - generating output values (usually
visual properties) based on a predefined function and changing input values.
Currently the main clients of cc/animation are Blink and ui/, targeting
composited layers, but the code is intended to be agnostic of the client it is
supporting. Aspirationally we could eventually merge cc/animation and Blink
animation and have only a single animation system for all of Chromium.

This document covers two main topics. The first section explains how
cc/animation actually works: how animations are ticked, what animation curves
are, what the ownership model is, etc. Later sections document how other parts
of Chromium interact with cc/animation, most prominently Blink and ui/.

## cc/animation Terminology

### Animation

An [Animation][] is responsible for managing and animating multiple properties
for a single target. A particular Animation may not be the sole Animation acting
on a given target. Animation is only a grouping mechanism for related
effects; the grouping relationship is defined by the client. It is also the
client's responsibility to deal with any conflicts that arise from animating
the same property of the same target across multiple Animations.

Each Animation has a copy on the impl thread, and will take care of
synchronizing to/from the impl thread when requested.

### KeyframeModel

[KeyframeModel][]s contain the state necessary to 'play' (i.e. interpolate
values from) an [AnimationCurve][], which is a function that returns a value
given an input time. Aside from the AnimationCurve itself, a KeyframeModel's
state includes the run state (playing, paused, etc), the start time, the current
direction (forwards, reverse), etc. It does not know or care what property is
being animated and holds only an opaque identifier for the property to allow
clients to map output values to the correct properties.

### KeyframeEffect

A [KeyframeEffect][] owns a group of KeyframeModels for a single target. It is
responsible for managing the KeyframeModels' running states (starting, running,
paused, etc), as well as ticking the KeyframeModels when it is requested to
produce new outputs for a given time. There is a 1:1 relationship between
Animation and KeyframeEffect.

Note that a single KeyframeEffect may not own all the KeyframeModels for a given
target. KeyframeEffect is only a grouping mechanism for related KeyframeModels.
All KeyframeModels for a given target can found via ElementAnimations - there
is only one ElementAnimations for a given target.

In general, KeyframeModels are grouped together in a KeyframeEffect and each
such group is owned by an Animation.

### Group

KeyframeModels that must be run together are called 'grouped' and have the same
group id. Grouped KeyframeModels are guaranteed to start at the same time and no
other KeyframeModels may animate any of the group's target properties until all
KeyframeModels in the group have finished animating. It's also guaranteed that
no two KeyframeModels within a KeyframeEffect that have both the same group id
and target property.

### Ticking An Animation

In order to play an Animation, input time values must be provided to the
AnimationCurve and output values fed back into the animating entity. This is
called 'ticking' an Animation and is the responsibility of the
[AnimationHost][]. The AnimationHost has a list of currently ticking Animations
(i.e. those that have any non-deleted KeyframeModels), which it iterates through
whenever it receives a tick call from the client (along with a corresponding
input time). The Animations then call into their non-deleted KeyframeModels,
retrieving a value from the AnimationCurve. As they are computed, output
values are sent to the target which is responsible for passing them to the
client entity that is being animated.

### Types of Animation Curve

As noted above, an AniationCurve is simply a function which converts an input
time value into some output value. AnimationCurves are categorized based on
their output type, and each such category can have multiple implementations that
provide different conversion functions. There are many categories of
AnimationCurve, but some common ones are `FloatAnimationCurve`,
`ColorAnimationCurve`, and `TransformAnimationCurve`.

The most common implementation of the various animation curve categories are the
[keyframed animation curves](https://source.chromium.org/chromium/chromium/src/+/main:ui/gfx/animation/keyframe/keyframed_animation_curve.h).
These curves each have a set of keyframes which map a specific time to a
specific output value. Producing an output value for a given input time is then
a matter of identifying the two keyframes the time lies between, and
interpolating between the keyframe output values. (Or simply using a keyframe
output value directly, if the input time happens to line up exactly.) Exact
details of how each animation curve category is interpolated can be found in the
implementations.

There is one category of animation curve that stands somewhat apart, the
[ScrollOffsetAnimationCurve][]. This curve converts the input time into a
scroll offset, interpolating between an initial scroll offset and an updateable
target scroll offset. It has logic to handle different types of scrolling such
as programmatic, keyboard, and mouse wheel scrolls.

### Animation Timelines

cc/animation has a concept of an [AnimationTimeline][]. This should not be
confused with the identically named Blink concept. In cc/animation,
AnimationTimelines are an implementation detail; they hold the Animations and
are responsible for syncing them to the impl thread (see below), but they do not
participate in the ticking process in any way.

### Main/Impl Threads

One part of cc/animation that is not client agnostic is its support for the
[Chromium compositor thread](https://codesearch.chromium.org/chromium/src/cc/README.md).
Most of the cc/animation classes have a `PushPropertiesTo` method, in which they
synchronize necessary state from the main thread to the impl thread. It is
feasible that such support could be abstracted if necessary, but so far it has
not been required.

## Current cc/animation Clients

As noted above, the main clients of cc/animation are currently Blink for
accelerated web animations, and ui/ for accelerated user interface animations.
Both of these clients utilize
[cc::Layer](https://codesearch.chromium.org/chromium/src/cc/layers/layer.h)
as their animation entity and interact with cc/animation via the
[MutatorHostClient](https://codesearch.chromium.org/chromium/src/cc/trees/mutator_host_client.h)
interface (which is implemented by cc::LayerTreeHost and cc::LayerTreeHostImpl).

chrome/browser/vr/ also makes use of cc/animations but does not use cc::Layer as
its animation entity.

### Supported Animatable Properties

As cc::Layers are just textures which are reused for performance, clients that
use composited layers as their animation entities are limited to animating
properties that do not cause content to be redrawn. For example, a composited
layer's opacity can be animated as promoted layers are aware of the content
behind them.  On the other hand we cannot animate layer width as changing the
width could modify layout - which then requires redrawing.

### Interaction between cc/animation and Blink

Blink is able to move compatible animations off the main thread by promoting
the animating element into a layer. The Blink
[Lifetime of a compositor animation](../../third_party/blink/renderer/core/animation/README.md#lifetime-of-a-compositor-animation)
document describes how composited animations are created in blink. Once a
compositor animation is created it is pushed through the commit cycle.

![new animation]

The lifetime of a newly started cc::Animation is roughly the following:

1. An update to style or a new animation triggers a new [BeginMainFrame][] via
   [ScheduleVisualUpdate][].
1. [blink::DocumentAnimations::UpdateAnimations][] calls [blink::Animation::PreCommit][]
   on each pending blink::Animation constructing the corresponding
   cc::Animation via [blink::Animation::CreateCompositorAnimation][] (attaching
   the animation to the cc::AnimationTimeline resulting in it being later pushed).
   The KeyframeEffects are constructed via [blink::Animation::StartAnimationOnCompositor][].
1. [cc::AnimationHost::RegisterAnimationForElement][] creates a
   cc::ElementAnimations for the target `element_id` if one does not already
   exist. This ElementAnimations instance is shared by all animations with
   the same target.
1. During the commit, [cc::LayerTreeHostImpl::FinishCommit][] calls
   [cc::LayerTreeImpl::PullPropertiesFrom][] which results in
   [cc::AnimationTimeline::PushAttachedAnimationsToImplThread][] creating a
   cc::Animation on the compositor thread's AnimationTimeline for each animation
   missing from the compositor thread.
1. [cc::Animation::PushPropertiesTo][] is called on every animation on the timeline.
   When the `element_id` is pushed by [cc::KeyframeEffect::PushPropertiesTo][]
   [cc::AnimationHost::RegisterAnimationForElement][] creates a compositor side
   cc::ElementAnimations instance. Since animations are pushed after the layer and property trees,
   the element should already exist on the pending tree. This will result in the
   animation being added to the ticking animations list.
1. Now the animation is ticking, meaning that [cc::Animation::Tick][] will be called
   every frame and update the pending property tree nodes.
1. When the pending tree is activated,
   [cc::AnimationHost::ActivateAnimations][] updates the keyframe effects.
1. Subsequent animation ticks will now update the property nodes on the active
   tree.

[new animation]: images/new-animation.png
[BeginMainFrame]: https://cs.chromium.org/chromium/src/cc/trees/proxy_main.cc?type=cs&q=file:proxy_main%5C.cc+RequestMainFrameUpdate
[ScheduleVisualUpdate]: https://cs.chromium.org/chromium/src/third_party/blink/renderer/core/frame/local_frame.cc?type=cs&q=file:local_frame%5C.cc+ScheduleVisualUpdate
[blink::DocumentAnimations::UpdateAnimations]: https://cs.chromium.org/search?q=function:blink::DocumentAnimations::UpdateAnimations+GetPendingAnimations
[blink::Animation::PreCommit]: https://cs.chromium.org/search?q=function:blink::PendingAnimations::Update+%5C-%5C>PreCommit%5C(&g=0&l=57
[blink::Animation::CreateCompositorAnimation]: https://cs.chromium.org/search?q=function:blink::Animation::CreateCompositorAnimation+%5E%5B+%5D*AttachCompositorTimeline
[blink::Animation::StartAnimationOnCompositor]: https://cs.chromium.org/search?q=function:blink::Animation::StartAnimationOnCompositor+%5C-%5C>StartAnimationOnCompositor
[cc::AnimationHost::RegisterAnimationForElement]: https://cs.chromium.org/search?q=function:cc::AnimationHost::RegisterAnimationForElement+ElementAnimations::Create
[cc::LayerTreeHostImpl::FinishCommit]: https://cs.chromium.org/search?q=cc::LayerTreeHostImpl::FinishCommit+file:%5C.cc
[cc::LayerTreeImpl::PullPropertiesFrom]: https://cs.chromium.org/search/?q=function:cc::LayerTreeHostImpl::FinishCommit+%5C-%5C>PullPropertiesFrom
[cc::AnimationTimeline::PushAttachedAnimationsToImplThread]: https://cs.chromium.org/search?q=function:cc::AnimationTimeline::PushAttachedAnimationsToImplThread+animation%5C-%5C>CreateImplInstance
[cc::Animation::PushPropertiesTo]: https://cs.chromium.org/search?q=cc::Animation::PushPropertiesTo+file:%5C.cc
[cc::KeyframeEffect::PushPropertiesTo]: https://cs.chromium.org/search?q=cc::KeyframeEffect::PushPropertiesTo+file:%5C.cc
[cc::AnimationHost::RegisterAnimationForElement]: https://cs.chromium.org/search?q=cc::AnimationHost::RegisterAnimationForElement+file:%5C.cc
[cc::Animation::Tick]: https://cs.chromium.org/search?q=cc::Animation::Tick+file:%5C.cc
[cc::AnimationHost::ActivateAnimations]: https://cs.chromium.org/search?q=cc::AnimationHost::ActivateAnimations+ActivateKeyframeModels
[KeyframeEffect]: https://cs.chromium.org/chromium/src/cc/animation/keyframe_effect.h
[PropertyToElementIdMap]: https://cs.chromium.org/chromium/src/cc/trees/target_property.h?type=cs&g=0&l=42

#### Animation Events
The purpose of AnimationEvents ([cc::AnimationEvent][], not to confused with
[blink::AnimationEvent][]) is to synchronize animation state from cc::Animation
to its client. The typical life cycle of the events is:
1. **Event Generation.**
Events are generated on IMPL thread and collected into [cc::AnimationEvents][]
container.  [cc::AnimationEvents][] are passed to the MAIN thread as part of
[BeginMainFrame][] arguments.
1. **Event Dispatch.**
On the MAIN thread, events are dispatched to [cc::KeyframeModel][]s to ensure
they are synchronized to their counterparts on the IMPL side. TIME_UPDATED
events skip this step since [cc::KeyframeModel][]s of worklet animations
don't participate in generating and reacting to these events.
1. **Event Delegation.**
After the events are dispatched, they are delegated to
[cc::AnimationDelegate][], the final destination of the events on cc:animation's
client.

There is a special type of event called impl_only events. These are generated by
Animations that don't have a counterpart on the MAIN thread. These events are
not passed to the MAIN thread and skip the dispatch stage. They are delegated to
the [cc::AnimationDelegate][] immediately from the IMPL thread.

[cc::AnimationEvent]: https://cs.chromium.org/chromium/src/cc/animation/animation_events.h
[cc::AnimationEvents]: https://cs.chromium.org/chromium/src/cc/animation/animation_events.h
[blink::AnimationEvent]: https://cs.chromium.org/chromium/src/third_party/blink/renderer/core/events/animation_event.h
[cc::KeyframeModel]: https://cs.chromium.org/chromium/src/cc/animation/keyframe_model.h
[cc::AnimationDelegate]: https://cs.chromium.org/chromium/src/cc/animation/animation_delegate.h

`TODO(flackr): Document finishing / cancel / abort.`

### Interaction between cc/animation and ui/

`TODO(smcgruer): Write this.`

## Additional References

The [blink/animations
README](https://chromium.googlesource.com/chromium/src/+/main/third_party/blink/renderer/core/animation/README.md)
contains comprehensive documentation about how Blink animations work and
integrate with CC animations.

The Compositor Property Trees talk [slides](https://goo.gl/U4wXpW)
includes discussion on compositor animations.

The Project Heaviside [design document](https://goo.gl/pWaWyv)
and [slides](https://goo.gl/iFpk4R) provide history on the Chromium
and Blink animation system. The slides in particular include helpful
software architecture diagrams.

Smooth scrolling is implemented via animations. See also references to
"scroll offset" animations in the cc code
base. [Smooth Scrolling in Chromium](https://goo.gl/XXwAwk) provides
an overview of smooth scrolling. There is further class header
documentation in
Blink's
[platform/scroll](https://codesearch.chromium.org/chromium/src/third_party/blink/renderer/platform/scroll/)
directory.
