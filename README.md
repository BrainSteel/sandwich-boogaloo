# Sandwich Boogaloo

## 2017 February 3-5 ICT Game Jam submission, at Wichita State University

### Team Members
Nathan Brandes: Art  
Brandon Jones: Programmer  
Jesse Pritchard: Programmer  
Steven Reust: Misc/Programmer/Art  

### Description
Sandwich Boogaloo is a sandwich construction game about racing against the clock to construct the mightiest sandwich you can.
Control the piece of bread with WASD and make it to the Crescent Roll at the right side of the level to advance to the next level.
On the way is an evil Sand Witch intent on getting in your way as best as she can.

The game features fully randomly generated levels that work most of the time.

The entire game was written between Friday, February 3 and Sunday, February 5, in C using no external engines or libraries.
The initial commit contains a makefile and some (ultimately broken) pixel level rendering routines prepared that morning.
The final commit on the master branch labeled FINAL GAME JAM VERSION is the product presented to the judges and community on February 5.

### Compilation
The game was compiled using MinGW 32-bit on Windows 10. If you are using MinGW, the game can be built simply by running `mingw32-make`
in the parent directory. The game can also theoretically be compiled using MinGW on a Mac using the `makefile-mac` makefile, though this 
is mostly untested.

The game makes extensive use of the Windows runtime and will not operate on any other operating system, though could be modified
to do so with some effort.

Your mileage with other compilers and other versions of Windows may vary.

Warning: A massive mess and game-breaking bugs abound. Tread carefully.
