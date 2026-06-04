# Chaotic Attractor Music Visualizer

An interactive music visualizer in which the positions of a set of points in 3D space are evolved using the differential equations of a chaotic attractor, with the instantaneous audio amplitude determining the integration time step.

## Features
- Real-time audio responsiveness
- Adaptive color changes based on the audio's amplitude
- Interactive XY plane rotation and zooming controls

## Chaotic Attractors
- Thomas Attractor
- Halvorsen Attractor
- Sprott Attractor
- Aizawa Attractor
- Lorenz Attractor

## Installation
- Clone the repository

  ```bash
  git clone https://github.com/natharyan/chaotic-attractors
  ```

### Install SFML(MacOS)
```bash
brew install sfml
```

- Get the location of the Graphics.hpp and Audio.hpp

  ```bash
  brew info sfml
  ```

  ```bash
  Output: /opt/homebrew/Cellar/sfml/2.6.1
  ```

- Check if Graphics.hpp and Audio.hpp are correctly installed

  ```bash
  ls /opt/homebrew/Cellar/sfml/2.6.1/include/SFML
  ```

- If you have a different installation path for SFML then replace the following in the MakeFile

  ```bash
  SFML_PATH = /opt/homebrew/Cellar/sfml/2.6.1
  ```

### Install SFML(Window)

- Follow this video: https://www.youtube.com/watch?v=Ptw1KKI9_Sg

## Usage

- Navigate to the repository on your terminal

  ```bash
  cd /path/to/chaotic-attractors
  ```

### Build (Makefile)

Build the project using the simple `Makefile`:

```bash
make            # builds (uses SFML_PATH default or override)
```

If your SFML installation is in a custom location, set `SFML_PATH`:

```bash
SFML_PATH=/path/to/sfml make
```

### Run

Run the built executable:

```bash
./bin/app
```

### Controls

- Click and drag your mouse to change the rotation of the visualizer along the x and y axis
- Press `T` to toggle the tails in the visualizer
- Use your arrow keys to change x and y offset of the screen
- Use your mouse scroll to change the scale of the visualizer screen
- Press `space` to pause the visualizer
- Use `R` to reset the visualizer configuration without restarting it
- Use `M` to toggle the stats menu
- Press `Q` to quit

## Customization

### Adjusting Audio Sensitivity
- Open the `cpp` file of an attractor in `src/includes/attractors`
- Reduce the maxAmplitude for increasing color response
- Modify the speedfactor formula for changing the particle speed response
  ```cpp
  float ThomasAttractor::speedfactor(float dt, float amplitude) const {
      return dt + 0.0001f * amplitude;
  }
  ```
### Color Palettes
- Modify the `startColor` and `endColor` variables in the  `cpp` file of an attractor in `src/includes/attractors`
  ```cpp
  startColor = sf::Color(70, 130, 180);
  endColor = sf::Color(239, 204, 144);
  ```
### Adding New Attractors
- Copy paste the header and `cpp` files of one of the already implemented attractor into new files
- In the header file, modify the attractor class name `AizawAttractor`
- In the header file, replace the constructor with the new class name `RosslerAttractor(float dt)`
- In the header file, replace the private variables with the parameters of the new attractor
- In the `cpp` file, change the class and constructor name `RosslerAttractor::RosslerAttractor(float dt) : Attractor()`
- In the `cpp` file, experiment around with different values of variables
- Add a new mp3 file to `./audio/` and change replace the `defaultaudio` value in the `cpp` file with the relative path of the new file with respect to your `pwd`
  ```bash
  defaultaudio = "audio/Gymnopedie.mp3"
  ```
- In the `cpp` file, change the `step` function to use your new attractor system
- In the `cpp` file, experiment with the `speedfactor` formula

Also check out this fun video on chaotic attractors: https://www.youtube.com/watch?v=uzJXeluCKMs&t=251s
