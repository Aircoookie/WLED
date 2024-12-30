# Preset Navigation usermod
This usermod lets you navigate a large number of effect presets more easily. You can group effects together however you see fit into *banks* that you can navigate either between (eg bank 1 to bank 2) or within (switch between presets inside a bank). It's particularly handy for non-touchscreen interfaces, like pushbuttons or encoders.

![A graphic showing three boxes, containing LED strips that each represent an effect preset. The first box contains 3 sets of LED strips of different hues but the same effect, labeled Bank 1. The next box shows only 2 LED strips with rainbow themes, and is labeled Bank 2. The final box shows 4 LED strips and a representation of different LED chase effects, labeled Bank 3.](PresetNavigationDiagram.png)

Commands are provided to cycle up or down, with and without wrapping.

## Using this usermod
Add the flag `-D USERMOD_PRESET_NAVIGATION` to your build flags and [compile WLED](https://kno.wled.ge/advanced/compiling-wled/), if you can't find firmware with this usermod already included. Flash your new firmware and now there will be a Preset Navigation section under Settings>Usermods. Be sure to check the "Enable" checkbox and enter your preset banks as described below.

## Preset notation
The graphical example above could be represented like this in the usermod settings, "Preset Banks" text area:

```
10,11,12
20,21
30,31,32,33
```

This is assuming you've defined presets for given effects with the ID numbers 10, 11, 12, 21... etc. The preset IDs do not have to be sequential or follow any pattern like this, but it's up to you to carefully input this information: the usermod currently doesn't do any checks here for undefined preset IDs. **Do not include trailing commas after banks, just a newline**. You can include a space after commas, but that just makes the stored string take up a little more space in memory.

## How it functions
After starting up WLED, the state will be Preset 10---you can set this preset to be applied at boot when creating it. One option is to cycle through banks. Cycling up will apply Preset 20, and if cycling up again, Preset 30. If a wrapping command is used, then cycling up a third time will cycle back to Preset 10. From Preset 10, the other option is to cycle within a bank. Cycling up will apply Preset 11 next. Cycling down with a preset wrapping command from 10 will apply Preset 12.


## API commands
The commands are sent at the top level of the [JSON state API](https://kno.wled.ge/interfaces/json-api/), under the key `"pn"` for Preset Navigation. Here are some of the possible commands, the same pattern applies for both banks and presets.

| Command | Result                            |
|---------|-----------------------------------|
| `b~`    | Increment bank with wrap around   |
| `b+`    | Increment bank without wrapping   |
| `p~-`   | Decrement preset with wrap around |
| `p-`    | Decrement preset without wrapping |

Personally, for the navigation commands I'm interested in, I've defined some presets and then assigned their IDs to [buttons](https://kno.wled.ge/features/macros/#buttons). I name them with an underscore at the start so they are sorted separately. One such preset looks like this in the interface:

![A screenshot showing a preset with name "\_bank_up" and API command {"pn" : "b~"}](ScreenshotBankUp.png)

## Maximum number of presets, banks
By default, you can store up to 64 presets in up to 23 banks. These numbers can be increased if you have the memory for it, by redefining `USERMOD_PRESET_NAVIGATION_MAX_PRESETS` and `USERMOD_PRESET_NAVIGATION_MAX_BANKS` respectively. This can be done with a build flag the same way that you added this usermod to the build, eg `-D USERMOD_PRESET_NAVIGATION_MAX_PRESETS=96`

## Other considerations
This usermod naively keeps track of the current position in the navigation array. If you use the navigation commands to get to Preset 20 in our example above, and then use the web interface to apply Preset 10, sending an Cycle Bank Up command will still go to Preset 30 as if the previous state was still 20.

I like to save presets with "include brightness" unchecked, so that changing the brightness is independent to the changing of effects. In fact, on my own WLED builds I change the default of that checkbox to unchecked, by editing the file `wled00/data/index.js` and rebuilding the web interface with npm before compiling.
