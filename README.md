## Title: Space Invaders

## YouTube Showcase: https://www.youtube.com/watch?v=STl3jHWrPoU

![Alt text](part1/media/screenshots/SpaceInvadersVictory.gif)
![Alt text](part1/media/screenshots/SpaceInvadersDefeat.gif)

This project was created in C++ using SDL and OpenGL.

### Controls

> **WASD** - Move <br>
> **Left Click** *or* **Spacebar** - Shoot <br>
> **R** - Reset <br>
> **C** - Toggle Collider Display

### Layouts

Want to try out other alien layouts? Alien spawning layouts are pulled randomly at runtime from the `part1/media/data/alien_layouts.txt` file. The format is as follows:
> Two numbers, `R` `C`, the number of rows and columns of the layout <br>
> `R` lines with `C` space-separated characters, where `0` represents a gap and `1` represents an alien. <br>
> When `R` or `C` is 0, the parser stops.
