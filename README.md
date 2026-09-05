# About
This is a small project that I made to help me visualize and play with how light bends at a macroscopic scale.

Features are currently limited, I was more concerned with getting this up and running as quickly as possible, but I do plan on refactoring everything to make it more user friendly.

# Controls
Shift + right click = place a new prism\
right click = aim light beam\
left click = position light beam origin\
right click on prism = rotate prism\
left click on prism = position prism\
delete on prism = remove prism\
scroll on prism = change size of prism\
control + scroll on prism = change refractive index of prism

note: refractive index is unbounded, so wierd things may happen if it goes negative.

# Attribution
Project is written in C++14 using the raylib C++ template found [here.](https://github.com/CapsCollective/raylib-cpp-starter)
All the original files of the template fall under Licence (b), raylib and the c++ binding library for raylib have their own Licences [here](https://github.com/raysan5/raylib/blob/ae50bfa2cc569c0f8d5bc4315d39db64005b1b08/LICENSE) and [here](https://github.com/RobLoach/raylib-cpp/blob/1ef7ddbba17d8e6391281ca5b15a7c3e6f197c78/LICENSE) respectively. everything else falls under Licence (a)