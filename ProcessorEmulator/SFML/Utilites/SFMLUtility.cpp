#include "SFMLUtility.h"

sf::Vector2f SFMLUtility::CastVector2iToFloat(sf::Vector2i in) {
	return sf::Vector2f(static_cast<float>(in.x), static_cast<float>(in.y));
}
sf::Vector2f SFMLUtility::CastVector2uToFloat(sf::Vector2u in) {
	return sf::Vector2f(static_cast<float>(in.x), static_cast<float>(in.y));
}
sf::String SFMLUtility::operator""_sf(const char* str, std::size_t size) {
	return sf::String::fromUtf8(str, str + size);
}