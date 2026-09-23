#pragma once

#include <iostream>
#include <optional>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace SFMLUtility {
	sf::Vector2f CastVector2iToFloat(sf::Vector2i in);
	sf::Vector2f CastVector2uToFloat(sf::Vector2u in);
}