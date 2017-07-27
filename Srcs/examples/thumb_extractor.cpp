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
    vector<uint32_t> tileIndexes;
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

vector<uint32_t> getTileIndexes(HevcImageFileReader &reader, uint32_t contextId) {
    ImageFileReaderInterface::DataVector data;
    ImageFileReaderInterface::IdVector masterItemIds;    
    vector<uint32_t> tileIndexes;
    reader.getItemListByType(contextId, "master", masterItemIds);
    for (const auto masterId : masterItemIds) {
        reader.getItemDataWithDecoderParameters(contextId, masterId, data);
        tileIndexes.push_back(masterId);
    } 
    return tileIndexes;
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

void writeMetadataToDisk(Metadata metadata) {
    json j;
    j["tile_indexes"] = metadata.tileIndexes;
    j["width"] = metadata.width;
    j["height"] = metadata.height;
    j["rotation"] = metadata.rotation;
    ofstream ofile("metadata.json");
    ofile << j;
    ofile.close();
}

void writeTilesToDisk(Metadata metadata) {
    for (size_t i = 0; i < metadata.tileIndexes.size(); i++) {
        char buff[100];
        snprintf(buff, sizeof(buff), "%d", metadata.tileIndexes[i]);
        string tilename = buff;        
        ofstream ofile(tilename);
        ofile.write((char*) &metadata.tileBlobs[i].data[0], metadata.tileBlobs[i].data.size());
        ofile.close();
    }
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

    metadata.tileIndexes = getTileIndexes(reader, contextId);

    metadata.tileBlobs = getTileBlobs(reader, contextId);

    if (metadata.tileIndexes.size() != metadata.tileBlobs.size()) {
        return 1;
    }

    writeMetadataToDisk(metadata);

    writeTilesToDisk(metadata);

    cout << "All done!" << endl;

    reader.close();
    return 0;
}



