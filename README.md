# immich-3ds

for all your lo-fi photograpy needs


## Building

- set up docker (or the devkitarm toolchain natively on your machine, but this
  kinda sucks and requires some slight makefile modifications) on your machine.
- edit `config.h` to point to your immich server & have your
  [immich api key](https://immich.app/docs/features/command-line-interface#obtain-the-api-key)
- `make`

## Installing

- [Jailbreak your 3ds](https://3ds.hacks.guide/)
- Make sure your 3ds is on the same network as the computer you're presumably on
- Open the homebrew launcher
- Press «Y»
- Right now I'm doing `make upload` with the proper toolchain installed. this
  uploads it remotely and launches it


## Using

Pressing right trigger with the app launched currently uploads all photos
stored within the sd card.  Make sure you're saving photos to the sd card or
transfer them over using the photo app.


## Planned features

- Run as a background service, polling for new files to upload
- Store state of previously pushed files somewhere to not deduplicate uploads
- Upload files with the hash to cancel early in case of duplicate file uploads
- Parse json returned by the immich server to do things more properly
    - parsing the .well-known url
    - doing a bulk upload check
- Read photos taken to SRAM
- Figure out timezone offsets / automatically apply timezone & offset



## Thanks

Thank you to my wonderful boyfriend kalin for helping me navigate the
retro homebrew ecosystem and jailbreaking my 3ds for me.

Also thank you to recurse center for helping me find the time and the structure
to make this project happen.

