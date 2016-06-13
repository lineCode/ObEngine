//Author : Sygmei
//Key : 976938ef7d46c286a2027d73f3a99467bcfa8ff0c1e10bd0016139744ef5404f4eb4d069709f9831f6de74a094944bf0f1c5bf89109e9855290336a66420376f

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <clip.h>

#include "Character.hpp"
#include "World.hpp"
#include "DataParser.hpp"
#include "Cursor.hpp"
#include "KeyBind.hpp"
#include "GUI.hpp"
#include "HUD.hpp"
#include "Dialog.hpp"
#include "Particle.hpp"
#include "Console.hpp"
#include "MathExp.hpp"
#include "Script.hpp"
#include "TimeManager.hpp"
#include "Dialog.hpp"
#include "Light.hpp"
#include "GameObject.hpp"
#include "EditorGrid.hpp"
#include "Serial.hpp"

namespace MapEditor
{
	class SpriteFile
	{
		private:
			std::string name;
			sf::Texture texture;
			sf::Font font;
		public:
			SpriteFile(std::string name, sf::Font font);
			std::string getName();
			void render(sf::RenderTexture* rtexture);
			sf::Texture getTexture();
	};

	class SpriteFolder
	{
		private:
			std::string category;
			std::string name;
			std::string folderIcon;
			sf::Texture texture;
			sf::Sprite sprite;
			sf::Font font;
			std::vector<SpriteFile*> sprList;
		public:
			SpriteFolder(std::string category, std::string name, std::string folderIcon, sf::Font font);
			void pushSprite(SpriteFile* spr);
			std::vector<SpriteFile*>* getSpriteList();
			void render(sf::RenderTexture* rtexture);
			sf::Sprite* getSprite();
			sf::Texture getTexture();
	};

	void loadSpriteTab(std::string geid);
	void buildAddSpriteFolderList();
	void displayAddSpriteFolderList();
	void addSpriteToWorld(std::string geid);
	void editMap(std::string mapName);
}