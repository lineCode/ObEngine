//Author : Sygmei
//Key : 976938ef7d46c286a2027d73f3a99467bcfa8ff0c1e10bd0016139744ef5404f4eb4d069709f9831f6de74a094944bf0f1c5bf89109e9855290336a66420376f

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <chrono>
#include <random>
#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <fstream>
#include <tinydir/tinydir.h>
#include <vili/ErrorHandler.hpp>

#include "Coordinates.hpp"

namespace obe
{
    namespace Functions
    {
        namespace Map
        {
            template <typename T, typename U>
            bool isInMap(T item, std::map<U, T>& map);
            template <typename T, typename U>
            bool keyInMap(T item, std::map<T, U>& map);
        }

        namespace Math
        {
            extern double pi;
            int randint(const int& min, const int& max);
            double randfloat();
            template <typename N>
            N getMin(const N& min1, const N& min2);
            template <typename N>
            N getMax(const N& max1, const N& max2);
            template <typename N, typename N1, typename N2>
            bool isBetween(const N& target, const N1& lowerBound, const N2& upperBound);
            bool isDoubleInt(const double& value);
            template <typename T> 
            int sign(T val);
        }

        namespace Run
        {
            class Parser
            {
            private:
                char** start;
                int size;
            public:
                Parser(char** start, int size);
                bool argumentExists(const std::string& arg) const;
                std::string getArgumentValue(const std::string& arg) const;
            };
        }

        namespace String
        {
            std::vector<std::string> split(const std::string& str, const std::string& delimiters = " ");
            std::vector<std::string> multiSplit(std::string str, const std::vector<std::string>& seps);
            int occurencesInString(const std::string& str, const std::string& occur);
            bool isStringAlpha(const std::string& str);
            bool isStringAlphaNumeric(const std::string& str);
            bool isStringNumeric(const std::string& str);
            bool isStringInt(const std::string& str);
            bool isStringFloat(const std::string& str);
            void removeCharFromString(std::string& str, const std::string& charToRemove);
            void replaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
            std::string replaceString(std::string subject, const std::string& search, const std::string& replace);
            bool isBetween(const std::string& string, const std::string& bet);
            std::string extract(const std::string& base, int start, int end);
            std::vector<std::string> extractBetween(std::string& str, char delimiter1, char delimiter2);
            std::string getRandomKey(std::string set, int len);
            void regenerateEncoding(std::string& str);
            std::string stringToAsciiCode(std::string& str);
            std::string cutBeforeAsciiCode(std::string& str, int asciiCode);
            typedef std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::pair<int, int>>> StringExtractor;
            StringExtractor extractAllStrings(std::string str);
            bool contains(const std::string& string, const std::string& search);
        }

        namespace Type
        {
            template <typename V>
            std::string getObjectType(V item);
            template <class T>
            std::string getClassType();
        }

        namespace Vector
        {
            template <typename V>
            bool isInList(V item, const std::vector<V>& vector);
            template <typename V>
            int indexOfElement(V item, const std::vector<V>& vector);
            std::string join(std::vector<std::string>& vector, std::string sep = "", int start = 0, int end = 0);
            template <typename V>
            void eraseAll(std::vector<V>& vector, V elem);
            void joinBetween(std::vector<std::string>& vector, std::string joinValue, std::string sep = "");
            void mergeNeighboors(std::vector<std::string>& vector, std::string n1, std::string n2, std::string sep = "", bool strict = false);
            template <typename V>
            int findSubVector(std::vector<V>& vector, std::vector<V>& find);
            template <typename V>
            std::vector<V> getSubVector(const std::vector<V>& vector, int start = 0, int end = 0);
        }

        //Functions::Map
        template <typename T, typename U>
        bool Map::isInMap(T item, std::map<U, T>& map)
        {
            for (auto iterator = map.begin(); iterator != map.end(); ++iterator)
            {
                if (iterator->second == item)
                    return true;
            }
            return false;
        }

        template <typename T, typename U>
        bool Map::keyInMap(T item, std::map<T, U>& map)
        {
            for (auto iterator = map.begin(); iterator != map.end(); ++iterator)
            {
                if (iterator->first == item)
                    return true;
            }
            return false;
        }

        //Functions::Math
        template <typename N>
        N Math::getMin(const N& min1, const N& min2)
        {
            return (min1 < min2) ? min1 : min2;
        }

        template <typename N>
        N Math::getMax(const N& max1, const N& max2)
        {
            return (max1 > max2) ? max1 : max2;
        }

        template <typename N, typename N1, typename N2>
        bool Math::isBetween(const N& target, const N1& lowerBound, const N2& upperBound)
        {
            if (target >= lowerBound && target <= upperBound)
                return true;
            return false;
        }

        template <typename T> 
        int Math::sign(T val)
        {
            return (T(0) < val) - (val < T(0));
        }

        //Functions::Type
        template <typename V>
        std::string Type::getObjectType(V item)
        {
            std::vector<std::string> splittedTypeName = String::split(typeid(item).name(), " ");
            return Vector::join(splittedTypeName, "", 1);
        }

        template <class T>
        std::string Type::getClassType()
        {
            std::vector<std::string> splittedTypeName = String::split(typeid(T).name(), " ");
            return Vector::join(splittedTypeName, "", 1);
        }

        //Functions::Vector
        template <typename V>
        bool Vector::isInList(V term, const std::vector<V>& list1)
        {
            for (size_t k = 0; k < list1.size(); k++)
            {
                if (term == list1[k])
                    return true;
            }
            return false;
        }

        template <typename V>
        int Vector::indexOfElement(V item, const std::vector<V>& vector)
        {
            for (int k = 0; k < vector.size(); k++)
            {
                std::cout << k << " >> : " << vector[k] << std::endl;
            }
            for (unsigned int k = 0; k < vector.size(); k++)
            {
                std::cout << "Compare : " << k << " : " << item << " / " << vector[k] << std::endl;
                if (item == vector[k])
                {
                    std::cout << "FoundItem -> Returning : " << k << std::endl;
                    return k;
                }
            }
            throw aube::ErrorHandler::Raise("ObEngine.FunctionsHeader.Vector.UnknownIndexOfElement", {{"element", std::to_string(item)}});
        }

        template <typename V>
        void Vector::eraseAll(std::vector<V>& vector, V elem)
        {
            std::vector<int> toErase;
            for (int i = 0; i < vector.size(); i++)
            {
                if (vector.at(i) == elem)
                    toErase.push_back(i);
            }
            for (int i = toErase.size() - 1; i >= 0; i--)
                vector.erase(vector.begin() + toErase[i]);
        }

        template <typename V>
        int Vector::findSubVector(std::vector<V>& vector, std::vector<V>& find)
        {
            int occ = 0;
            int potentialIndex = -1;
            bool startedSearch = false;
            if (find.size() < vector.size())
            {
                for (int i = 0; i < vector.size(); i++)
                {
                    std::cout << "Compare : " << find[occ] << " and " << vector[i] << std::endl;
                    if (find[occ] == vector[i])
                    {
                        if (!startedSearch)
                        {
                            potentialIndex = i;
                            startedSearch = true;
                        }
                        occ++;
                    }
                    else if (startedSearch && find[occ] == vector[i])
                    {
                        startedSearch = false;
                        occ = 0;
                        potentialIndex = -1;
                    }
                }
            }
            return potentialIndex;
        }

        template <typename V>
        std::vector<V> Vector::getSubVector(const std::vector<V>& vector, int start, int end)
        {
            auto first = vector.begin() + start;
            auto last = vector.end() - end;
            std::vector<V> newVec(first, last);
            return newVec;
        }
    }
}
