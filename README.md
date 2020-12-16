# AsyncTimer
JavaScript like Async timing functions (setTimeout, setInterval)

## Installing

#### Aduino IDE:

Download the repository as .zip and include it as a new library into the IDE selecting the menu:

```
 Sketch -> include Library -> Add .Zip library
```

#### platformIO:

Go to libraries and type `AsyncTimer` into the search bar and add it to your project.

## Getting Started
Simply include the library into your sketch and make one instance of `AsyncTimer` and add the setup function to `void setup()` and the handler to `void loop()` and then start using it!

## Example:

```c++
#include <AsyncTimer.h>

AsyncTimer t;

void setup()
{
  t.setup();
}

void loop()
{
  t.handle();
}
```

### NOTE: only one instance must be created and it must be outside any function, as shown in the example above.

# API

## setTimeout(callbackFunction, delayInMs)

`setTimeout` takes two arguments, the first one is the function to call after waiting, the second one is the time in milliseconds to wait before executing the function. It returns an `unsigned short` id of the timeout.
It will run only once unless canceled.

#### Example:

- Using lambda function:

```c++
AsyncTimer t;

t.setTimeout([&]() {
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

`setInterval` takes the same parameters as `setTimeout` and returns an `unsigned short` id of the interval, unlike `setTimeout`, it will keep executing the code forever unless canceled.

#### Example:

- Using lambda function:

```c++
AsyncTimer t;

t.setInterval([&]() {
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

## cancel(intervalOrTimeoutId)

It cancels the execution of a timeout or an interval.

`cancel` takes one argument, the `id` returned from `setTimeout` or `setInterval` function and returns `void`.

#### Example:

- Cancelling an interval:

```c++
AsyncTimer t;

unsigned short intervalId = t.setInterval([&]() {
  Serial.println("Hello world!");
}, 2000);

// Cancel the interval after 7 seconds:
t.setTimeout([&]() {
  t.cancel(intervalId);
}, 7000);
```

- Cancelling a timeout:

```c++
AsyncTimer t;

// This timeout will never run
unsigned short timeoutId = t.setTimeout([&]() {
  Serial.println("Hello world!");
}, 3000);

// Cancel the timeout before it's executed
t.cancel(timeoutId);
```

# Examples

- BlinkUsingInterval - Blink led using `setInterval`.
- SerialMsgUsingTimeout - Send a message to the serial monitor using `setTimeout` 10 seconds after booting.
- CancelInterval - Cancel an interval using `cancel`.
- CancelTimeout - Cancel a timeout using `cancel`.

# License

This library is licensed under `MIT license`.