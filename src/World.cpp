//Author : Sygmei
//Key : 976938ef7d46c286a2027d73f3a99467bcfa8ff0c1e10bd0016139744ef5404f4eb4d069709f9831f6de74a094944bf0f1c5bf89109e9855290336a66420376f

#include "World.hpp"

namespace obe
{
    namespace World
    {
        World::World()
        {
            m_worldScriptEngine = new kaguya::State();
            loadWorldScriptEngineBaseLib(m_worldScriptEngine);
            (*m_worldScriptEngine)["World"] = this;
            System::Path("Lib/GameLib/WScrInit.lua").loadResource(m_worldScriptEngine, System::Loaders::luaLoader);
            Script::loadLib(m_worldScriptEngine, "Core.*");
            Script::TriggerDatabase::GetInstance()->createNamespace("Map");
            m_showCollisionModes["drawLines"] = false;
            m_showCollisionModes["drawPoints"] = false;
            m_showCollisionModes["drawMasterPoint"] = false;
            m_showCollisionModes["drawSkel"] = false;
        }

        World::~World()
        {
            Script::TriggerDatabase::GetInstance()->removeNamespace("Map");
        }

        Graphics::LevelSprite* World::createLevelSprite(std::string id)
        {
            std::unique_ptr<Graphics::LevelSprite> newLevelSprite = std::make_unique<Graphics::LevelSprite>(id);

            Graphics::LevelSprite* returnLevelSprite = newLevelSprite.get();
            m_spriteArray.push_back(move(newLevelSprite));

            this->reorganizeLayers();
            return returnLevelSprite;
        }

        Collision::PolygonalCollider* World::createCollider(std::string id)
        {
            std::unique_ptr<Collision::PolygonalCollider> newCollider = std::make_unique<Collision::PolygonalCollider>(id);
            Collision::PolygonalCollider* returnCollider = newCollider.get();
            m_colliderArray.push_back(move(newCollider));
            return returnCollider;
        }

        void World::addLight(Light::PointLight* lgt)
        {
            m_lightMap[lgt->getID()] = std::unique_ptr<Light::PointLight>(lgt);
        }

        kaguya::State* World::getScriptEngine() const
        {
            return m_worldScriptEngine;
        }

        std::string World::getBaseFolder() const
        {
            return m_baseFolder;
        }

        void World::loadFromFile(std::string filename)
        {
            this->clearWorld();
            vili::DataParser mapParse;
            m_baseFolder = System::Path("Data/Maps").add(filename).loadResource(&mapParse, System::Loaders::dataLoader);
            std::cout << "Base Folder : " << m_baseFolder << std::endl;

            if (mapParse->contains(vili::AttributeType::ComplexAttribute, "Meta"))
            {
                vili::ComplexAttribute& meta = mapParse.at("Meta");
                m_levelName = meta.getBaseAttribute("name").get<std::string>();
            }
            else
                throw aube::ErrorHandler::Raise("ObEngine.World.World.NoMeta", {{"map", filename}});

            if (mapParse->contains(vili::AttributeType::ComplexAttribute, "View"))
            {
                vili::ComplexAttribute& view = mapParse.at("View");
                m_camera.setSize(view.at<vili::BaseAttribute>("size").get<double>());
                m_cameraInitialPosition = Coord::UnitVector(
                    view.at<vili::BaseAttribute>("pos", "x").get<double>(),
                    view.at<vili::BaseAttribute>("pos", "y").get<double>(),
                    Coord::stringToUnits(view.at<vili::BaseAttribute>("pos", "unit").get<std::string>()));
            }
            else
                throw aube::ErrorHandler::Raise("ObEngine.World.World.NoView", {{"map", filename}});

            if (mapParse->contains(vili::AttributeType::ComplexAttribute, "LevelSprites"))
            {
                vili::ComplexAttribute& levelSprites = mapParse.at("LevelSprites");
                for (std::string& currentSpriteName : levelSprites.getAll(vili::AttributeType::ComplexAttribute))
                {
                    vili::ComplexAttribute& currentSprite = levelSprites.at(currentSpriteName);
                    std::vector<std::string> spriteAtrList;
                    std::string spriteID = currentSpriteName;
                    std::string spriteUnits = currentSprite.contains(vili::AttributeType::ComplexAttribute, "rect") ?
                                                  currentSprite.at<vili::BaseAttribute>("rect", "unit").get<std::string>() : "WorldUnits";
                    std::cout << "SpriteUnit : " << spriteUnits << std::endl;
                    std::string spritePath = currentSprite.contains(vili::AttributeType::BaseAttribute, "path") ?
                                                 currentSprite.getBaseAttribute("path").get<std::string>() : "";
                    Coord::UnitVector spritePos(0, 0);
                    Coord::UnitVector spriteSize(1, 1);
                    if (currentSprite.contains(vili::AttributeType::ComplexAttribute, "rect"))
                    {
                        Coord::Units rectUnit = Coord::stringToUnits(spriteUnits);
                        spritePos.unit = rectUnit;
                        spritePos.x = currentSprite.at<vili::BaseAttribute>("rect", "x").get<double>();
                        spritePos.y = currentSprite.at<vili::BaseAttribute>("rect", "y").get<double>();
                        spriteSize.unit = rectUnit;
                        spriteSize.x = currentSprite.at<vili::BaseAttribute>("rect", "w").get<double>();
                        spriteSize.y = currentSprite.at<vili::BaseAttribute>("rect", "h").get<double>();
                        spritePos = spritePos.to<Coord::Units::WorldUnits>();
                        spriteSize = spriteSize.to<Coord::Units::WorldUnits>();
                    }
                    int spriteRot = currentSprite.contains(vili::AttributeType::BaseAttribute, "rotation") ?
                                        currentSprite.getBaseAttribute("rotation").get<int>() : 0;
                    int layer = currentSprite.contains(vili::AttributeType::BaseAttribute, "layer") ?
                                    currentSprite.getBaseAttribute("layer").get<int>() : 1;
                    int zdepth = currentSprite.contains(vili::AttributeType::BaseAttribute, "z-depth") ?
                                     currentSprite.getBaseAttribute("z-depth").get<int>() : 1;

                    if (currentSprite.contains(vili::AttributeType::ListAttribute, "attributeList"))
                    {
                        for (vili::BaseAttribute* attribute : currentSprite.at<vili::ListAttribute>("attributeList"))
                            spriteAtrList.push_back(*attribute);
                    }

                    std::unique_ptr<Graphics::LevelSprite> tempSprite = std::make_unique<Graphics::LevelSprite>(spriteID);
                    if (spritePath != "")
                        tempSprite->load(spritePath);
                    tempSprite->setPosition(spritePos.x, spritePos.y);
                    tempSprite->setSize(spriteSize.x, spriteSize.y);
                    tempSprite->setWorkingUnit(Coord::stringToUnits(spriteUnits));
                    tempSprite->setRotation(spriteRot);
                    tempSprite->setAtr(spriteAtrList);
                    tempSprite->setLayer(layer);
                    tempSprite->setZDepth(zdepth);
                    m_spriteArray.push_back(move(tempSprite));
                }
            }

            this->reorganizeLayers();

            if (mapParse->contains(vili::AttributeType::ComplexAttribute, "Collisions"))
            {
                vili::ComplexAttribute& collisions = mapParse.at("Collisions");
                for (std::string& collisionName : collisions.getAll(vili::AttributeType::ComplexAttribute))
                {
                    vili::ComplexAttribute& currentCollision = collisions.at(collisionName);
                    std::cout << "New Collider : " << collisionName << std::endl;
                    std::unique_ptr<Collision::PolygonalCollider> tempCollider = std::make_unique<Collision::PolygonalCollider>(collisionName);

                    std::string pointsUnit = currentCollision.at<vili::BaseAttribute>("unit", "unit").get<std::string>();
                    bool completePoint = true;
                    double pointBuffer;
                    Coord::Units pBaseUnit = Coord::stringToUnits(pointsUnit);
                    for (vili::BaseAttribute* colliderPoint : currentCollision.getListAttribute("points"))
                    {
                        if ((completePoint = !completePoint))
                        {
                            Coord::UnitVector pVector2 = Coord::UnitVector(
                                pointBuffer,
                                colliderPoint->get<double>(),
                                pBaseUnit
                            ).to<Coord::Units::WorldPixels>();
                            std::cout << "Add Vector Pt : " << pVector2 << std::endl;
                            tempCollider->addPoint(pVector2.x, pVector2.y);
                        }
                        else
                            pointBuffer = colliderPoint->get<double>();
                    }
                    tempCollider->setWorkingUnit(pBaseUnit);
                    m_colliderArray.push_back(move(tempCollider));
                }
            }

            if (mapParse->contains(vili::AttributeType::ComplexAttribute, "LevelObjects"))
            {
                vili::ComplexAttribute& levelObjects = mapParse.at("LevelObjects");
                for (std::string& currentObjectName : levelObjects.getAll(vili::AttributeType::ComplexAttribute))
                {
                    vili::ComplexAttribute& currentObject = levelObjects.at(currentObjectName);
                    std::string levelObjectType = currentObject.getBaseAttribute("type").get<std::string>();
                    this->createGameObject(currentObjectName, levelObjectType);
                    if (currentObject.contains(vili::AttributeType::ComplexAttribute, "Requires"))
                    {
                        vili::ComplexAttribute& objectRequirements = currentObject.at("Requires");
                        currentObject.removeOwnership(&objectRequirements);
                        Script::GameObjectRequires::ApplyRequirements(this->getGameObject(currentObjectName), objectRequirements);
                        objectRequirements.setParent(&currentObject);
                    }
                }
            }

            if (mapParse->contains(vili::AttributeType::ComplexAttribute, "Script"))
            {
                vili::ComplexAttribute& script = mapParse.at("Script");
                for (vili::BaseAttribute* scriptName : script.getListAttribute("gameScripts"))
                {
                    System::Path(*scriptName).loadResource(m_worldScriptEngine, System::Loaders::luaLoader);
                    m_scriptArray.push_back(*scriptName);
                }
            }
        }

        void World::clearWorld()
        {
            m_spriteArray.clear();
            m_colliderArray.clear();
            m_gameObjectMap.clear();
            m_updateObjArray.clear();
            m_lightMap.clear();
            m_scriptArray.clear();
        }

        vili::DataParser* World::saveData()
        {
            vili::DataParser* dataStore = new vili::DataParser;
            dataStore->createFlag("Map");
            dataStore->createFlag("Lock");
            dataStore->includeFile("Obe");

            //Meta
            (*dataStore)->createComplexAttribute("Meta");
            dataStore->at("Meta").createBaseAttribute("name", m_levelName);

            //View
            (*dataStore)->createComplexAttribute("View");
            dataStore->at("View").createBaseAttribute("size", m_camera.getSize().y / 2);
            dataStore->at("View").createComplexAttribute("pos");
            dataStore->at("View", "pos").createBaseAttribute("unit", unitsToString(m_cameraInitialPosition.unit));
            dataStore->at("View", "pos").createBaseAttribute("x", m_cameraInitialPosition.x);
            dataStore->at("View", "pos").createBaseAttribute("y", m_cameraInitialPosition.y);
            dataStore->at("View", "pos").useTemplate(dataStore->getTemplate("Vector2<WorldUnits>"));

            //LevelSprites
            if (m_spriteArray.size() > 0) (*dataStore)->createComplexAttribute("LevelSprites");
            for (unsigned int i = 0; i < m_spriteArray.size(); i++)
            {
                if (m_spriteArray[i]->getParentID() == "")
                {
                    dataStore->at("LevelSprites").createComplexAttribute(m_spriteArray[i]->getID());
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID()).createBaseAttribute("path", m_spriteArray[i]->getPath());
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID()).createComplexAttribute("rect");
                    Coord::UnitVector spritePositionRect = m_spriteArray[i]->getPosition().to<Coord::Units::WorldUnits>(/*m_spriteArray[i]->getWorkingUnit()*/);
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID(), "rect").createBaseAttribute("x", spritePositionRect.x);
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID(), "rect").createBaseAttribute("y", spritePositionRect.y);
                    Coord::UnitVector spriteSizeRect = Coord::UnitVector(
                        m_spriteArray[i]->getSpriteWidth(), 
                        m_spriteArray[i]->getSpriteHeight(), 
                        Coord::Units::WorldPixels).to<Coord::Units::WorldUnits>(/*m_spriteArray[i]->getWorkingUnit()*/);
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID(), "rect").createBaseAttribute("w", spriteSizeRect.x);
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID(), "rect").createBaseAttribute("h", spriteSizeRect.y);
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID(), "rect").useTemplate(
                        dataStore->getTemplate("Rect<" + unitsToString(m_spriteArray[i]->getWorkingUnit()) + ">")
                    );
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID()).createBaseAttribute("rotation", m_spriteArray[i]->getRotation());
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID()).createBaseAttribute("layer", m_spriteArray[i]->getLayer());
                    dataStore->at("LevelSprites", m_spriteArray[i]->getID()).createBaseAttribute("z-depth", m_spriteArray[i]->getZDepth());
                    if (m_spriteArray[i]->getAttributes().size() != 0)
                    {
                        dataStore->at("LevelSprites", m_spriteArray[i]->getID()).createListAttribute("attributeList");
                        for (unsigned int j = 0; j < m_spriteArray[i]->getAttributes().size(); j++)
                            dataStore->at("LevelSprites", m_spriteArray[i]->getID()).getListAttribute("attributeList").push(m_spriteArray[i]->getAttributes()[j]);
                    }
                }
            }
            if (m_colliderArray.size() > 0) (*dataStore)->createComplexAttribute("Collisions");
            for (unsigned int i = 0; i < m_colliderArray.size(); i++)
            {
                if (m_colliderArray[i]->getParentID() == "")
                {
                    dataStore->at("Collisions").createComplexAttribute(m_colliderArray[i]->getID());
                    dataStore->at("Collisions", m_colliderArray[i]->getID()).createComplexAttribute("unit");
                    dataStore->at("Collisions", m_colliderArray[i]->getID(), "unit").useTemplate(
                        dataStore->getTemplate("Unit<" + unitsToString(m_colliderArray[i]->getWorkingUnit()) + ">")
                    );
                    dataStore->at("Collisions", m_colliderArray[i]->getID()).createListAttribute("points");
                    for (unsigned int j = 0; j < m_colliderArray[i]->getPointsAmount(); j++)
                    {
                        Coord::UnitVector pVec = m_colliderArray[i]->u_getPointPosition(j);
                        dataStore->at("Collisions", m_colliderArray[i]->getID()).getListAttribute("points").push(pVec.x);
                        dataStore->at("Collisions", m_colliderArray[i]->getID()).getListAttribute("points").push(pVec.y);
                    }
                }
            }
            if (m_gameObjectMap.size() > 0) (*dataStore)->createComplexAttribute("LevelObjects");
            for (auto it = m_gameObjectMap.begin(); it != m_gameObjectMap.end(); ++it)
            {
                dataStore->at("LevelObjects").createComplexAttribute(it->first);
                dataStore->at("LevelObjects", it->first).createBaseAttribute("type", it->second->getType());
                (*it->second->m_objectScript)("inspect = require('Lib/StdLib/Inspect');");
                kaguya::LuaRef saveTableRef = (*it->second->m_objectScript)["Local"]["Save"]();
                (*it->second->m_objectScript)("print('Saving : ', This:getID())");
                (*it->second->m_objectScript)("print(inspect(Local.Save()));");
                (*it->second->m_objectScript)("print('Processing...')");
                vili::ComplexAttribute* saveRequirements = Script::DataBridge::luaTableToComplexAttribute(
                    "Requires", saveTableRef);
                if (saveRequirements->getAll().size() > 0)
                    dataStore->at("LevelObjects", it->first).pushComplexAttribute(saveRequirements);
                (*it->second->m_objectScript)("print('Saving over for : ', This:getID())");
            }
            if (m_scriptArray.size() > 0)
            {
                (*dataStore)->createComplexAttribute("Script");
                dataStore->at("Script").createListAttribute("gameScripts");
                for (int i = 0; i < m_scriptArray.size(); i++)
                {
                    dataStore->at("Script").getListAttribute("gameScripts").push(m_scriptArray[i]);
                }
            }
            return dataStore;
        }

        void World::update(double dt)
        {
            if (m_cameraLocked)
                m_camera.lock();
            else
                m_camera.unlock();
            if (m_updateState)
            {
                m_gameSpeed = dt;
                if (m_needToOrderUpdateArray)
                {
                    this->orderUpdateScrArray();
                    m_needToOrderUpdateArray = false;
                }

                for (int i = 0; i < m_updateObjArray.size(); i++)
                {
                    if (!m_updateObjArray[i]->deletable)
                        m_updateObjArray[i]->update(dt);
                    else
                    {
                        //BUGGY
                        /*if (updateObjArray[i]->hasCollider)
                            this->deleteCollision(updateObjArray[i]->getCollider(), false);*/
                        if (m_updateObjArray[i]->m_hasLevelSprite)
                            this->deleteSprite(m_updateObjArray[i]->getLevelSprite());
                        m_updateObjArray.erase(m_updateObjArray.begin() + i);
                    }
                }

                for (auto iterator = m_lightMap.begin(); iterator != m_lightMap.end(); ++iterator)
                {
                    if (iterator->second->getType() == "Dynamic")
                    {
                        dynamic_cast<Light::DynamicPointLight*>(iterator->second.get())->updateLight();
                    }
                }
            }
            m_camera.unlock();
        }

        void World::display(sf::RenderWindow& target)
        {
            this->displaySprites(target);
            if (m_showCollisionModes["drawLines"] || m_showCollisionModes["drawPoints"] || m_showCollisionModes["drawMasterPoint"] || m_showCollisionModes["drawSkel"])
            {
                for (unsigned int i = 0; i < m_colliderArray.size(); i++)
                {
                    m_colliderArray[i]->setDrawOffset(-m_camera.getPosition().to<Coord::Units::WorldPixels>().x, -m_camera.getPosition().to<Coord::Units::WorldPixels>().y);
                    m_colliderArray[i]->draw(target, m_showCollisionModes["drawLines"],
                                             m_showCollisionModes["drawPoints"],
                                             m_showCollisionModes["drawMasterPoint"],
                                             m_showCollisionModes["drawSkel"]);
                }
            }
        }

        void World::displaySprites(sf::RenderWindow& target)
        {
            Coord::UnitVector pixelCamera = m_camera.getPosition().to<Coord::Units::WorldPixels>();
            for (unsigned int i = 0; i < m_spriteArray.size(); i++)
            {
                Coord::UnitVector spritePosition = m_spriteArray[i]->getDrawPosition(pixelCamera);
                sfe::ComplexSprite& tAffSpr = m_spriteArray[i]->getSprite();

                //std::cout << "Draw Position of Sprite : " << spritePosition << std::endl;

                tAffSpr.setPosition(spritePosition.x, spritePosition.y);
                //tAffSpr.setScalingOrigin(-layeredX, -layeredY); Work on this later :)
                //tAffSpr.scale(m_camera.getHeight() / 2, m_camera.getHeight() / 2);
                if (m_spriteArray[i]->isVisible())
                {
                    target.draw(tAffSpr);
                    //if (m_spriteArray[i]->isSelected())
                    m_spriteArray[i]->drawHandle(target, spritePosition.x, spritePosition.y);
                }
            }
        }

        std::string World::getLevelName() const
        {
            return m_levelName;
        }

        void World::setLevelName(std::string newName)
        {
            m_levelName = newName;
        }

        std::vector<Collision::PolygonalCollider*> World::getColliders() const
        {
            std::vector<Collision::PolygonalCollider*> allColliders;
            for (auto& collider : m_colliderArray)
                allColliders.push_back(collider.get());
            return allColliders;
        }

        Camera* World::getCamera()
        {
            return &m_camera;
        }

        void World::setCameraLock(bool state)
        {
            m_cameraLocked = state;
        }

        bool World::isCameraLocked() const
        {
            return m_cameraLocked;
        }

        void World::setUpdateState(bool state)
        {
            m_updateState = state;
        }

        Script::GameObject* World::getGameObject(std::string id)
        {
            if (m_gameObjectMap.find(id) != m_gameObjectMap.end())
                return m_gameObjectMap[id].get();
            throw aube::ErrorHandler::Raise("ObEngine.World.World.UnknownGameObject", {{"id", id}, {"map", m_levelName}});
        }

        std::vector<Script::GameObject*> World::getAllGameObjects(std::vector<std::string> filters)
        {
            std::vector<Script::GameObject*> returnVec;
            for (auto it = m_gameObjectMap.begin(); it != m_gameObjectMap.end(); ++it)
            {
                if (filters.size() == 0) returnVec.push_back(it->second.get());
                else
                {
                    bool allFilters = true;
                    if (Functions::Vector::isInList(std::string("Display"), filters)) { if (!it->second->canDisplay()) allFilters = false; }
                    if (Functions::Vector::isInList(std::string("Collide"), filters)) { if (!it->second->canCollide()) allFilters = false; }
                    if (Functions::Vector::isInList(std::string("Click"), filters)) { if (!it->second->canClick()) allFilters = false; }
                    if (allFilters) returnVec.push_back(it->second.get());
                }
            }
            return returnVec;
        }

        Script::GameObject* World::createGameObject(std::string id, std::string obj)
        {
            std::unique_ptr<Script::GameObject> newGameObject = std::make_unique<Script::GameObject>(obj, id);
            vili::DataParser getGameObjectFile;
            System::Path("Data/GameObjects/").add(obj).add(obj + ".obj.vili").loadResource(&getGameObjectFile, System::Loaders::dataLoader);
            vili::ComplexAttribute& gameObjectData = getGameObjectFile.at(obj);
            newGameObject->loadGameObject(*this, gameObjectData);
            if (newGameObject->m_hasScriptEngine)
            {
                loadWorldLib(newGameObject->m_objectScript.get());
                (*newGameObject.get()->m_objectScript)["World"] = this;
            }

            if (newGameObject->canDisplay())
            {
                if (newGameObject->canDisplay() && newGameObject->isLevelSpriteRelative())
                    newGameObject->getLevelSprite()->setPosition(0, 0);
                newGameObject->getLevelSprite()->setParentID(id);
            }
            if (newGameObject->m_hasCollider)
            {
                newGameObject->getCollider()->setParentID(id);
            }

            m_gameObjectMap[id] = move(newGameObject);
            m_needToOrderUpdateArray = true;

            std::cout << "<World> Created new object : " << id << " of type : " << obj << std::endl;

            return m_gameObjectMap[id].get();
        }

        void World::orderUpdateScrArray()
        {
            m_updateObjArray.clear();
            for (auto it = m_gameObjectMap.begin(); it != m_gameObjectMap.end(); ++it)
                m_updateObjArray.push_back(it->second.get());
            sort(m_updateObjArray.begin(), m_updateObjArray.end(), Script::orderScrPriority);
        }

        void World::reorganizeLayers()
        {
            bool noChange = false;
            while (noChange == false)
            {
                noChange = true;
                for (unsigned int i = 0; i < m_spriteArray.size(); i++)
                {
                    if (i != m_spriteArray.size() - 1 && m_spriteArray[i]->getLayer() < m_spriteArray[i + 1]->getLayer())
                    {
                        swap(m_spriteArray[i], m_spriteArray[i + 1]);
                        noChange = false;
                    }
                }
            }
            noChange = false;
            while (noChange == false)
            {
                noChange = true;
                for (unsigned int i = 0; i < m_spriteArray.size(); i++)
                {
                    if (i != m_spriteArray.size() - 1 && m_spriteArray[i]->getLayer() == m_spriteArray[i + 1]->getLayer() && m_spriteArray[i]->getZDepth() < m_spriteArray[i + 1]->getZDepth())
                    {
                        swap(m_spriteArray[i], m_spriteArray[i + 1]);
                        noChange = false;
                    }
                }
            }
        }

        Graphics::LevelSprite* World::getSpriteByIndex(int index)
        {
            return m_spriteArray[index].get();
        }

        int World::getSpriteArraySize() const
        {
            return m_spriteArray.size();
        }

        std::vector<Graphics::LevelSprite*> World::getAllSprites()
        {
            std::vector<Graphics::LevelSprite*> allSprites;
            for (int i = 0; i < m_spriteArray.size(); i++)
                allSprites.push_back(m_spriteArray[i].get());
            return allSprites;
        }

        std::vector<Graphics::LevelSprite*> World::getSpritesByLayer(int layer)
        {
            std::vector<Graphics::LevelSprite*> returnLayer;

            for (unsigned int i = 0; i < m_spriteArray.size(); i++)
            {
                if (m_spriteArray[i]->getLayer() == layer)
                    returnLayer.push_back(m_spriteArray[i].get());
            }

            return returnLayer;
        }

        Graphics::LevelSprite* World::getSpriteByPos(int x, int y, int layer)
        {
            Graphics::LevelSprite* returnSpr = nullptr;
            std::vector<Graphics::LevelSprite*> getSpriteVec = getSpritesByLayer(layer);
            for (unsigned int i = 0; i < getSpriteVec.size(); i++)
            {
                if (x > getSpriteVec[i]->getRect().left && x < getSpriteVec[i]->getRect().left + getSpriteVec[i]->getSpriteWidth())
                {
                    if (y > getSpriteVec[i]->getRect().top && y < getSpriteVec[i]->getRect().top + getSpriteVec[i]->getSpriteHeight())
                        returnSpr = getSpriteVec[i];
                }
            }
            return returnSpr;
        }

        Graphics::LevelSprite* World::getSpriteByID(std::string id)
        {
            for (int i = 0; i < m_spriteArray.size(); i++)
            {
                if (m_spriteArray[i].get()->getID() == id)
                    return m_spriteArray[i].get();
            }
            throw aube::ErrorHandler::Raise("ObEngine.World.World.UnknownLevelSprite", {{"id", id}, {"map", m_levelName}});
        }

        bool World::doesSpriteExists(const std::string& id)
        {
            for (int i = 0; i < m_spriteArray.size(); i++)
            {
                if (m_spriteArray[i].get()->getID() == id)
                    return true;
            }
            return false;
        }

        void World::deleteSpriteByID(std::string sprID)
        {
            this->deleteSprite(this->getSpriteByID(sprID));
        }

        void World::deleteSprite(Graphics::LevelSprite* spriteToDelete)
        {
            for (int i = 0; i < m_spriteArray.size(); i++)
            {
                if (m_spriteArray[i].get()->getID() == spriteToDelete->getID())
                    m_spriteArray.erase(m_spriteArray.begin() + i);
            }
        }

        void World::enableShowCollision(bool drawLines, bool drawPoints, bool drawMasterPoint, bool drawSkel)
        {
            m_showCollisionModes["drawLines"] = drawLines;
            m_showCollisionModes["drawPoints"] = drawPoints;
            m_showCollisionModes["drawMasterPoint"] = drawMasterPoint;
            m_showCollisionModes["drawSkel"] = drawSkel;
        }

        std::pair<Collision::PolygonalCollider*, int> World::getCollisionPointByPos(int x, int y)
        {
            for (unsigned int i = 0; i < m_colliderArray.size(); i++)
            {
                if (m_colliderArray[i]->hasPoint(x, y, 6, 6) != -1)
                {
                    return std::pair<Collision::PolygonalCollider*, int>(m_colliderArray[i].get(), m_colliderArray[i]->hasPoint(x, y, 6, 6));
                }
            }
            return std::pair<Collision::PolygonalCollider*, int>(nullptr, 0);
        }

        Collision::PolygonalCollider* World::getCollisionMasterByPos(int x, int y)
        {
            for (unsigned int i = 0; i < m_colliderArray.size(); i++)
            {
                if (m_colliderArray[i]->hasMasterPoint(x, y, 6, 6))
                    return m_colliderArray[i].get();
            }
            return nullptr;
        }

        Collision::PolygonalCollider* World::getCollisionByID(std::string id)
        {
            for (unsigned int i = 0; i < m_colliderArray.size(); i++)
            {
                if (id == m_colliderArray[i]->getID())
                {
                    return m_colliderArray[i].get();
                }
            }
            return nullptr;
        }

        std::vector<Collision::PolygonalCollider*> World::getAllCollidersByCollision(Collision::PolygonalCollider* col, int offx, int offy)
        {
            std::vector<Collision::PolygonalCollider*> returnVec;
            for (int i = 0; i < m_colliderArray.size(); i++)
            {
                if (m_colliderArray[i]->doesCollide(col, offx, offy))
                {
                    returnVec.push_back(m_colliderArray[i].get());
                }
            }
            return returnVec;
        }

        void World::deleteCollisionByID(std::string id)
        {
            this->deleteCollision(this->getCollisionByID(id));
        }

        void World::deleteCollision(Collision::PolygonalCollider* colToDelete)
        {
            int indexToDelete = 0;
            for (unsigned int i = 0; i < m_colliderArray.size(); i++)
            {
                if (colToDelete == m_colliderArray[i].get())
                    indexToDelete = i;
            }
            m_colliderArray.erase(m_colliderArray.begin() + indexToDelete);
        }

        void World::createCollisionAtPos(int x, int y)
        {
            int i = 0;
            std::string testID = "collider" + std::to_string(m_colliderArray.size() + i);
            while (getCollisionByID(testID) != nullptr)
            {
                ++i;
                testID = "collider" + std::to_string(m_colliderArray.size() + i);
            }
            Collision::PolygonalCollider* newCollider = this->createCollider(testID);
            newCollider->addPoint(50, 0);
            newCollider->addPoint(0, 50);
            newCollider->addPoint(100, 50);
            newCollider->setPositionFromMaster(x, y);
        }

        void loadWorldLib(kaguya::State* lua)
        {
            if (!static_cast<bool>((*lua)["Core"])) (*lua)["Core"] = kaguya::NewTable();
            (*lua)["Core"]["World"] = kaguya::NewTable();
            (*lua)["Core"]["World"]["World"].setClass(kaguya::UserdataMetatable<World>()
                .addFunction("addLight", &World::addLight)
                .addFunction("addCollider", &World::createCollider)
                .addFunction("clearWorld", &World::clearWorld)
                .addFunction("createCollisionAtPos", &World::createCollisionAtPos)
                .addFunction("createGameObject", &World::createGameObject)
                .addFunction("createLevelSprite", &World::createLevelSprite)
                .addFunction("deleteCollisionByID", &World::deleteCollisionByID)
                .addFunction("deleteSprite", &World::deleteSprite)
                .addFunction("enableShowCollision", &World::enableShowCollision)
                .addFunction("getAllCollidersByCollision", &World::getAllCollidersByCollision)
                .addFunction("getAllGameObjects", &World::getAllGameObjects)
                .addFunction("getAllSprites", &World::getAllSprites)
                .addFunction("getCamera", &World::getCamera)
                .addFunction("getColliders", &World::getColliders)
                .addFunction("getCollisionByID", &World::getCollisionByID)
                .addFunction("getCollisionMasterByPos", &World::getCollisionMasterByPos)
                .addFunction("getCollisionPointByPos", &World::getCollisionPointByPos)
                .addFunction("getGameObject", &World::getGameObject)
                .addFunction("getScriptEngine", &World::getScriptEngine)
                .addFunction("getSpriteArraySize", &World::getSpriteArraySize)
                .addFunction("getSpriteByID", &World::getSpriteByID)
                .addFunction("getSpriteByIndex", &World::getSpriteByIndex)
                .addFunction("getSpriteByPos", &World::getSpriteByPos)
                .addFunction("getSpritesByLayer", &World::getSpritesByLayer)
                .addFunction("loadFromFile", &World::loadFromFile)
                .addFunction("orderUpdateScrArray", &World::orderUpdateScrArray)
                .addFunction("reorganizeLayers", &World::reorganizeLayers)
                .addFunction("saveData", &World::saveData)
                .addFunction("setUpdateState", &World::setUpdateState)
            );
        }

        void loadWorldScriptEngineBaseLib(kaguya::State* lua)
        {
            (*lua)["CPP_Import"] = &Script::loadLib;
            (*lua)["CPP_Hook"] = &Script::loadHook;
            loadWorldLib(lua);
            Script::loadScrGameObjectLib(lua);
            (*lua)["This"] = lua;
        }
    };
};
