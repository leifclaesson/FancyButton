# This Library is DEPRECATED

[Use SuperButtons instead](https://github.com/leifclaesson/SuperButtons)
Similar functionality but _universal_, no need to pre-define each button. Just feed the output to an MQTT topic and go.



# FancyButton
Fancy button handler to add features like double and multi-presses. Can be used with any type of button but originally designed for 433 MHz remotes.

Only a single line of code is required per physical button.
Then, every time you get a (32-bit) code from the remote receiver, you feed it to all your FancyButton objects. The rest happens automatically.
You'll also need to call the Maintenance function of each FancyButton object a few times a second, but it doesn't have to happen for _every_ loop.

## Button Modes

### single

One event at a time.

Threshold `thresh` means how many codes need to see before we activate, i.e. how long you have to hold the button, and `timeout` specifies the silence period in milliseconds req'd before we're ready to activate again). The threshold function is appropriate for things where you really don't want spurious activation or deactivation, for example controlling a pump.
With a `thresh` of 1, this mode is great to toggle a simple light switch.


### repeat

Hold to repeat.

Instant activation on the first code, then we repeat activation for every `thresh` time the code is seen again, until `timeout` milliseconds of silence elapses. We also count and return the number of codes.
Appropriate as a volume control or a dimmer.



### tally

Press X times for action Y, or Hold for action 0.

Counts the number of pushes. Reports each push AND the final tally.
Works similarly to Fibaro's "The Button" Z-Wave scene controller.

The "each push" report is great for aural feedback if you're controlling a sound system. I use this mode to choose which internet radio stream to listen to! Obviously my favorite stations take 1 and 2 pushes.


### dual

Instantly reports when pushed, and reports again if code is repeated `thresh` times by holding the button.
I use this to control my ceiling fans.
Each push instantly bumps the speed 0-1-2-3 in a round-robin fashion. Holding the button sets speed 0.

The first push is INSTANTLY reported. This means that if the fans are on speed 2, and I hold the button to turn them off, they will briefly be set to speed 3 before the hold registers.
This mode works this may because I want the instant feedback. If the spurious activation is a problem, use _tally_ instead of this mode.


### unfiltered

Each matching code is reported straight through.


See the example sketch for more information.




## Dependencies

No direct dependencies, but you'll need to feed it unique button codes from somewhere, for example the [rc-switch](https://github.com/sui77/rc-switch) library for decoding 433 MHz remotes.
