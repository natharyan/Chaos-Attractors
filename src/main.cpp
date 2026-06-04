#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "includes/attractors/attractors.h"
#include "includes/attractors/base_attractor.h"
#include "includes/audio_player.h"
#include "includes/visualization.h"

int main() {
  AudioPlayer audioPlayer;
  std::string attractorchoice;
  std::unique_ptr<Attractor> attractor;
  std::string title;

  sf::VideoMode desktopMode = sf::VideoMode::getFullscreenModes()[0];
  std::cout << std::endl
            << "==== Chaotic Attractor Music Visualizer ====" << std::endl;
  std::cout << "Available Attractors:" << std::endl;
  std::cout << "1. Thomas" << std::endl;
  std::cout << "2. Halvorsen" << std::endl;
  std::cout << "3. Sprott" << std::endl;
  std::cout << "4. Aizawa" << std::endl;
  std::cout << "5. Lorenz" << std::endl;
  std::cout << "Enter the name of an attractor: ";
  std::cin >> attractorchoice;

  if (attractorchoice == "Lorenz") {
    attractor = std::make_unique<LorenzAttractor>(lorenz_defdt);
    title = "Lorenz Attractor";
  } else if (attractorchoice == "Aizawa") {
    attractor = std::make_unique<AizawaAttractor>(aizawa_defdt);
    title = "Aizawa Attractor";
  } else if (attractorchoice == "Thomas") {
    attractor = std::make_unique<ThomasAttractor>(thomas_defdt);
    title = "Thomas Attractor";
  } else if (attractorchoice == "Halvorsen") {
    attractor = std::make_unique<HalvorsenAttractor>(halvorsen_defdt);
    title = "Halvorsen Attractor";
  } else if (attractorchoice == "Sprott") {
    attractor = std::make_unique<SprottAttractor>(sprott_defdt);
    title = "Sprott Attractor";
  } else {
    std::cout << "Invalid attractor choice. Please try again.";
    return 1;
  }

  if (!audioPlayer.loadAndPlay(attractor->defaultaudio)) {
    std::cerr << "Error loading audio" << std::endl;
    return 1;
  }

  Visualization vis(desktopMode.size.x, desktopMode.size.y, title, audioPlayer,
                    *attractor);
  vis.run(*attractor);
  if (audioPlayer.sound)
    audioPlayer.sound->stop();

  return 0;
}