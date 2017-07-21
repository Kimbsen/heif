/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

/** This file includes some examples about using the reader API
 *  Note that binary execution fails if proper .heic files are not located in the directory */

#include "hevcimagefilereader.hpp"
#include <iostream>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

struct Blob {
    vector<unsigned char> data;
};

struct Metadata {
    uint32_t width;
    uint32_t height;
    uint16_t rotation;
    vector<string> tileNames;
    vector<Blob> tileBlobs;
};

vector<char> readFromStdin() {
    char input[1024];
    size_t readBufSize;
    vector<char> buffer;
    bool done = false;
    while (!done) {
        cin.read(input, sizeof(input));
        readBufSize = cin.gcount();
        if(cin.fail() || cin.bad() || cin.eof()) {
            if (std::cin.eof()) {
                done = true;
            }
            cin.clear();
        }
        for (size_t i = 0; i < readBufSize; i++) {
            buffer.push_back(input[i]);
        }
    }
    return buffer;
}

Metadata fetchMetadata(HevcImageFileReader &reader, uint32_t contextId) {
    ImageFileReaderInterface::GridItem gridItem;
    ImageFileReaderInterface::IdVector gridItemIds;    
    Metadata metadata = {0,0,0};

    //get all grid items
    reader.getItemListByType(contextId, "grid", gridItemIds);
    gridItem = reader.getItemGrid(contextId, gridItemIds.at(0));
    metadata.width = gridItem.outputWidth;
    metadata.height = gridItem.outputHeight;

    const uint32_t itemId = gridItemIds.at(0);
    const auto itemProperties =  reader.getItemProperties(contextId, itemId);
    for (const auto& property : itemProperties) {
        // For example, handle 'irot' transformational property is anti-clockwise rotation
        if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT) {
            metadata.rotation = reader.getPropertyIrot(contextId, property.index).rotation;
        }
    }

    return metadata;
}

vector<string> getTileNames(HevcImageFileReader &reader, uint32_t contextId) {
    ImageFileReaderInterface::DataVector data;
    ImageFileReaderInterface::IdVector masterItemIds;    
    vector<string> tileNames;
    reader.getItemListByType(contextId, "master", masterItemIds);
    for (const auto masterId : masterItemIds) {
        reader.getItemDataWithDecoderParameters(contextId, masterId, data);

        char buff[100];
        snprintf(buff, sizeof(buff), "tile_%d.h265", masterId);
        string filename = buff;
        tileNames.push_back(filename);
    } 
    return tileNames;
}

vector<Blob> getTileBlobs(HevcImageFileReader &reader, uint32_t contextId) {
    ImageFileReaderInterface::DataVector data;
    ImageFileReaderInterface::IdVector masterItemIds;    
    vector<Blob> tileBlobs;
    reader.getItemListByType(contextId, "master", masterItemIds);
    for (const auto masterId : masterItemIds) {
        reader.getItemDataWithDecoderParameters(contextId, masterId, data);
        Blob blob;
        blob.data = data;
        tileBlobs.push_back(blob);
    } 
    return tileBlobs;
}


int main() {
    auto buffer = readFromStdin();
    
    HevcImageFileReader reader;
    

    boost::interprocess::bufferstream input_stream(&buffer[0], buffer.size());
    reader.initialize(input_stream);

    // Verify that the file has one or several images in the MetaBox
    const auto& properties = reader.getFileProperties();
    if (!properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) && 
        !properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection) && 
        !properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageSequence)) {
        return 1;
    }
    
    // Find the item ID of the first master image
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    auto metadata = fetchMetadata(reader, contextId);

    cout << metadata.rotation << endl;
    cout << metadata.width << endl;
    cout << metadata.height << endl;


    metadata.tileNames = getTileNames(reader, contextId);

    metadata.tileBlobs = getTileBlobs(reader, contextId);

    for (auto tileName : metadata.tileNames) {
        cout << tileName << endl;
    }

    for (auto tileBlob : metadata.tileBlobs) {
        cout << tileBlob.data.size() << endl;
    }    

    json j;
    j["tile_names"] = metadata.tileNames;
    j["width"] = metadata.width;
    j["height"] = metadata.height;
    j["rotation"] = metadata.rotation;
    ofstream ofile("metadata.json");
    ofile << j;
    ofile.close();

    

    //write blob to stdout
    /*for (size_t i = 0; i < data.size(); i++) {
        cout << data[i];
    }*/
        


    reader.close();
    return 0;
}



