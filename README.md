# AsyncTimer

[![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncTimer.svg?)](https://www.ardu-badge.com/AsyncTimer)
[![GitHub release](https://img.shields.io/github/release/Aasim-A/AsyncTimer.svg)](https://github.com/Aasim-A/AsyncTimer/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/Aasim-A/AsyncTimer/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/Aasim-A/AsyncTimer.svg)](http://github.com/Aasim-A/AsyncTimer/issues)

### JavaScript-like Async timing functions (setTimeout, setInterval) for Arduino, ESP8266, ESP32 and other compatible boards

# Installing

## Aduino IDE:

#### Library Manager:

The easiest way is to install it through Arduino Library manager selecting the menu:
```
Sketch -> Include Library -> Manage Libraries
```
Then type `AsyncTimer` into the search box and install the latest version.

#### Manual install:

Download the repository as .zip and include it as a new library into the IDE selecting the menu:

```
 Sketch -> Include Library -> Add .Zip library
```

## PlatformIO:

Go to libraries and type [AsyncTimer](https://platformio.org/lib/show/11569/AsyncTimer) into the search bar and add it to your project.


# Getting Started
Simply include the library into your sketch and make one instance of `AsyncTimer` ~~and add the setup function to `void setup()`~~ and the handler to `void loop()` and then start using it!


#### Example:

```c++
#include <AsyncTimer.h>

AsyncTimer t;

void loop()
{
  t.handle();
}
```

> ⚠NOTE⚠: only one instance must be created and it must be outside any function, as shown in the example above.

> ⚠NOTE⚠: The default timer capacity is 10, if you wish to increase it, you have to use non-default initializer:

```c++
AsyncTimer t(22);
```

# API

## setTimeout(callbackFunction, delayInMs)

Creates a new timeout.

`setTimeout` takes two arguments, the first one is the function to call after waiting, the second one is the time in milliseconds to wait before executing the function. It returns an `unsigned short` id of the timeout. If the `timeout` creation was unseccussfull, it returns `0`.
It will run only once unless canceled.

#### Example:

- Using lambda function:

```c++
AsyncTimer t;

t.setTimeout([]() {
  Serial.println("Hello world!");
}, 2000);
// "Hello world!" will be printed to the Serial once after 2 seconds
```

- Using normal function:

```c++
AsyncTimer t;

void functionToCall()
{
  Serial.println("Hello world!");
}

t.setTimeout(functionToCall, 2000);
// "Hello world!" will be printed to the Serial once after 2 seconds
```

## setInterval(callbackFunction, delayInMs)

Creates a new interval.

`setInterval` takes the same parameters as `setTimeout` and returns an `unsigned short` id of the interval, unlike `setTimeout`, it will keep executing the code forever unless canceled. If the `interval` creation was unseccussfull, it returns `0`.

#### Example:

- Using lambda function:

```c++
AsyncTimer t;

t.setInterval([]() {
  Serial.println("Hello world!");
}, 2000);
// "Hello world!" will be printed to the Serial every 2 seconds
```

- Using normal function:

```c++
AsyncTimer t;

void functionToCall()
{
  Serial.println("Hello world!");
}

t.setInterval(functionToCall, 2000);
// "Hello world!" will be printed to the Serial every 2 seconds
```

## getRemaining(intervalOrTimeoutId)

Gets the number of milliseconds remaining in a timer. Returns `0` if the timer is not found.

`getRemaining` takes one argument, the `id` returned from `setTimeout` or `setInterval` function, returns `unsigned long`.

#### Example:

```c++
AsyncTimer t;

unsigned short timeoutId = t.setTimeout([]() {
  Serial.println("Hello world!");
}, 3000);

// Get the remaining ms on the timer
unsigned long remaining = t.getRemaining(timeoutId);
```

## changeDelay(intervalOrTimeoutId, delayInMs)
Changes the delay value of an active `intervalOrTimeout`.

`changeDelay` takes two arguments, the `id` returned from `setTimeout` or `setInterval` function and the new `delayValue` in `ms`, returns `void`.

#### Example:

- Changing the delay of `setInterval`:

```c++
AsyncTimer t;

unsigned short intervalId = t.setInterval([]() {
  Serial.println("Hello world!");
}, 2000);

t.setTimeout([]() {
  t.changeDelay(intervalId, 3500);
  // Now the interval runs every 3500ms instead of the old 2000ms
}, 7000);
```

## delay(intervalOrTimeoutId, delayInMs)
Delays the execution of an active `intervalOrTimeout`.

`delay` takes two arguments, the `id` returned from `setTimeout` or `setInterval` function and the new `delayValue` in `ms`, returns `void`.

#### Example:

- Delaying the execution of `setInterval`:

```c++
AsyncTimer t;

unsigned short intervalId = t.setInterval([]() {
  Serial.println("Hello world!");
}, 2000);

t.setTimeout([]() {
  t.delay(intervalId, 3500);
  // Now the interval will be delayed by an extra 3500ms,
  // afterwords, it will continue executing normally.
}, 7000);
```

## reset(intervalOrTimeoutId)
Resets the wait time of an active `intervalOrTimeout`.

`delay` takes one argument, the `id` returned from `setTimeout` or `setInterval` function, returns `void`.

#### Example:

- Resetting the wait time of `setInterval`:

```c++
AsyncTimer t;

unsigned short intervalId = t.setInterval([]() {
  Serial.println("Hello world!");
}, 2000);

t.setTimeout([]() {
  t.reset(intervalId);
  // Now the interval will be reset, this means that it will
  // execute exactly 2000ms after the reset function call.
}, 7000);
```

## cancel(intervalOrTimeoutId)

Cancels the execution of a timeout or an interval.

`cancel` takes one argument, the `id` returned from `setTimeout` or `setInterval` function and returns `void`.

#### Example:

- Cancelling an interval:

```c++
AsyncTimer t;

unsigned short intervalId = t.setInterval([]() {
  Serial.println("Hello world!");
}, 2000);

// Cancel the interval after 7 seconds:
t.setTimeout([]() {
  t.cancel(intervalId);
}, 7000);
```

- Cancelling a timeout:

```c++
AsyncTimer t;

// This timeout will never run
unsigned short timeoutId = t.setTimeout([]() {
  Serial.println("Hello world!");
}, 3000);

// Cancel the timeout before it's executed
t.cancel(timeoutId);
```

## cancelAll(includeIntervals = true)

Cancels the execution of a all timeouts and intervals or just timeouts.

`cancelAll` takes one optional argument, if you intend to only cancel timeouts, then call the function with `false`, otherwise it clears both timeouts and intervals be default, returns `void`.

#### Example:

- Cancelling all timeouts and intervals:

```c++
AsyncTimer t;

t.setInterval([]() {
  Serial.println("foo");
}, 2000);

t.setTimeout([]() {
  Serial.println("bar");
}, 7000);

t.setTimeout([]() {
  Serial.println("baz");
}, 7000);

// After this call, nothing will be running inside AsyncTimer
t.cancelAll();
```

- Only cancelling timeouts:

```c++
AsyncTimer t;

t.setInterval([]() {
  Serial.println("foo");
}, 2000);

t.setTimeout([]() {
  Serial.println("bar");
}, 7000);

t.setTimeout([]() {
  Serial.println("baz");
}, 7000);

// After this call, only intervals will be running inside AsyncTimer
t.cancelAll(false);
```

# Limitations
- Capturing lambda functions do not work.

# Examples

- BlinkUsingInterval - Blink led using `setInterval`.
- SerialMsgUsingTimeout - Send a message to the serial monitor using `setTimeout` 10 seconds after booting.
- CancelInterval - Cancel an interval using `cancel`.
- CancelTimeout - Cancel a timeout using `cancel`.
- DebounceUsingTimeout - Debounce button using a `delay`.

# License

This library is licensed under [MIT](https://github.com/Aasim-A/AsyncTimer/blob/master/LICENSE).

# Copyright

Copyright 2022 - Aasim-A
