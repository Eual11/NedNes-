- when loading nestest, the palette isn't correct!
- there are alot of problems with the rendering, completely not working
- the palette has been fixed, i guess...
- the rendering works a bit but its all nuts, it still not correct
- i guess the problem is with loopy registers and address fetching, i don't think the correct namespaces are even addressed in case of nes test for example, my emulator LITERALLY fails miserably even though the task was as simple as reading each tile from nametable and displaying it on screen
- the problem was how i set up the loopy register (because i didn't understand bit fields) the size of bit fields was 8 bit meanwhile loopy registers should have a size of 16 bits
- what is missing now is in other emulators i could see super mario bros editing the palette to make the coin appear shining, but mine isn't doing that 
- it fails miserably at scanlines test

