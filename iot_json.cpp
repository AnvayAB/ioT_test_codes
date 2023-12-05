#include <iostream>
#include <fstream>
#include <json/json.h>
#include <pugixml.hpp>

void jsonToXml(const Json::Value& jsonValue, pugi::xml_node& xmlNode) {
    if (jsonValue.isObject()) {
        for (const auto& key : jsonValue.getMemberNames()) {
            const Json::Value& value = jsonValue[key];
            pugi::xml_node childNode = xmlNode.append_child(key.c_str());
            jsonToXml(value, childNode);
        }
    } else if (jsonValue.isArray()) {
        for (int i = 0; i < jsonValue.size(); ++i) {
            pugi::xml_node childNode = xmlNode.append_child("item");
            jsonToXml(jsonValue[i], childNode);
        }
    } else {
        xmlNode.text().set(jsonValue.asString().c_str());
    }
}

int main() {
    // Read JSON from file (replace "path/to/your/input.json" with the actual JSON file path)
    std::ifstream jsonFile("path/to/your/input.json");
    if (!jsonFile.is_open()) {
        std::cerr << "Error opening JSON file" << std::endl;
        return 1;
    }

    Json::CharReaderBuilder jsonReader;
    Json::Value jsonValue;
    Json::parseFromStream(jsonReader, jsonFile, &jsonValue, nullptr);

    // Create an XML document
    pugi::xml_document xmlDoc;

    // Convert JSON to XML
    jsonToXml(jsonValue, xmlDoc);

    // Save the XML to file (replace "path/to/your/output.xml" with the desired XML file path)
    if (!xmlDoc.save_file("path/to/your/output.xml")) {
        std::cerr << "Error saving XML file" << std::endl;
        return 1;
    }

    std::cout << "Conversion successful. XML saved to 'path/to/your/output.xml'" << std::endl;

    return 0;
}