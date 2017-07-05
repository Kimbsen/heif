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

using namespace std;

/// Access and read image item and its thumbnail
int extractMaster() {
    char input[1024];
    size_t readBufSize;
    std::vector<char> buffer;
    bool done = false;
    while (!done) {
        std::cin.read(input, sizeof(input));
        readBufSize = cin.gcount();
        if(cin.fail() || cin.bad() || cin.eof()) {
            if (cin.eof()) {
                done = true;
            }
            std::cin.clear();
        }
        for (std::size_t i = 0; i < readBufSize; i++) {
            buffer.push_back(input[i]);
        }
    }
    
    HevcImageFileReader reader;
    ImageFileReaderInterface::DataVector data;
    ImageFileReaderInterface::DataVector dataTemp;
    ImageFileReaderInterface::IdVector itemIds;

    boost::interprocess::bufferstream input_stream(&buffer[0], buffer.size());
    reader.initialize(input_stream);

    const auto& properties = reader.getFileProperties();
    // Verify that the file has one or several images in the MetaBox
    if (!properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) && 
        !properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection) && 
        !properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageSequence)) {
        //std::cout << "HOLA1";
        return 1;
    }

    // Find the item ID of the first master image
 

    const auto& metaBoxFeatures = properties.rootLevelMetaBoxProperties.metaBoxFeature; // For convenience
    if (metaBoxFeatures.hasFeature(ImageFileReaderInterface::MetaBoxFeature::HasMasterImages)) {
        const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
        reader.getItemListByType(contextId, "master", itemIds);
        const uint32_t masterId = itemIds.at(0);   

        //std::cout << "HOLA2";
        reader.getItemDataWithDecoderParameters(contextId, masterId, data);
    } else if (metaBoxFeatures.hasFeature(ImageFileReaderInterface::MetaBoxFeature::HasThumbnails)) {
        std::cout << "HOLA3";
        return 1;
        /*const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
        reader.getItemListByType(contextId, "master", itemIds);
        const uint32_t masterId = itemIds.at(0);   

        reader.getReferencedToItemListByType(contextId, masterId, "thmb", itemIds);
        const uint32_t thumbnailId = itemIds.at(0);
        reader.getItemDataWithDecoderParameters(contextId, thumbnailId, data);*/
    } else {
        // std::cout << "HOLA4";
        // return 1;
        for (const auto& trackProperties : properties.trackProperties) {
            /*ImageFileReaderInterface::TrackFeature trackFeature = trackProperties.trackFeature;
            if (!trackFeature.HasSingleImage && !!trackFeature.HasImageCollection && !trackFeature.HasImageSequence) {
                continue;
            }*/
            const uint32_t contextId = trackProperties.first;
            //std::cout << "Track ID " << contextId << endl; // Context ID corresponds to the track ID

            for (const auto& sampleProperties : trackProperties.second.sampleProperties) {
                ImageFileReaderInterface::IdVector itemsToDecode;
                const uint32_t sampleId = sampleProperties.first;

                // A sample might have decoding dependencies. The simples way to handle this is just to always ask and
                // decode all dependencies.
                reader.getItemDecodeDependencies(contextId, sampleId, itemsToDecode);
                for (auto dependencyId : itemsToDecode) {
                    //std::cout << "HOLA1\n";
                    reader.getItemDataWithDecoderParameters(contextId, dependencyId, dataTemp);
                    for (size_t i = 0; i < dataTemp.size(); i++) {
                        data.push_back(dataTemp[i]);
                    }                    
                    dataTemp.clear();
                    // Feed data to decoder...
                }
            // Store or show the image...
            }
        }
    }
    //write blob to stdout
    for (std::size_t i = 0; i < data.size(); i++) {
        std::cout << data[i];
    }
    
    return 0;
}

/// Access and read a cover image
void example1(const std::string testFile)
{
    std::cout << "example1 started\n";

    HevcImageFileReader reader;
    reader.initialize(testFile);
    const auto& properties = reader.getFileProperties();

    // Verify that the file has a cover image
    if (not properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasCoverImage))
    {
        return; // Nothing to do, just return.
    }

    // The cover image is always located in the root level MetaBox, so get MetaBox context ID.
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    // Find the item ID
    const uint32_t itemId = reader.getCoverImageItemId(contextId);

    ImageFileReaderInterface::DataVector data;
    reader.getItemDataWithDecoderParameters(contextId, itemId, data);

    // Feed 'data' to decoder and display the cover image...
    std::cout << "example1 ended\n";
}

/// Access and read image item and its thumbnail
void example2(const std::string testFile)
{
    std::cout << "example2 started\n";

    HevcImageFileReader reader;
    ImageFileReaderInterface::DataVector data;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize(testFile);
    const auto& properties = reader.getFileProperties();

    // Verify that the file has one or several images in the MetaBox
    if (not (properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) ||
        properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection)))
    {
        return;
    }

    // Find the item ID of the first master image
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "master", itemIds);
    const uint32_t masterId = itemIds.at(0);

    const auto& metaBoxFeatures = properties.rootLevelMetaBoxProperties.metaBoxFeature; // For convenience
    if (metaBoxFeatures.hasFeature(ImageFileReaderInterface::MetaBoxFeature::HasThumbnails))
    {
        // Thumbnail references ('thmb') are from the thumbnail image to the master image
        reader.getReferencedToItemListByType(contextId, masterId, "thmb", itemIds);
        const uint32_t thumbnailId = itemIds.at(0);

        reader.getItemDataWithDecoderParameters(contextId, thumbnailId, data);
        // ...decode data and display the image, show master image later
    }
    else
    {
        // There was no thumbnail, show just the master image
        reader.getItemDataWithDecoderParameters(contextId, masterId, data);
        // ...decode and display...
    }
    std::cout << "example2 ended\n";
}

/// Access and read image items and their thumbnails in a collection
void example3(const std::string testFile)
{
    std::cout << "example3 started\n";

    HevcImageFileReader reader;
    ImageFileReaderInterface::DataVector data;
    typedef uint32_t MasterItemId;
    typedef uint32_t ThumbnailItemId;
    ImageFileReaderInterface::IdVector itemIds;
    map<MasterItemId, ThumbnailItemId> imageMap;

    reader.initialize(testFile);
    const auto& properties = reader.getFileProperties();

    // Verify that the file has one or several images in the MetaBox
    if (not (properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) ||
        properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection)))
    {
        return;
    }    

    // Find item IDs of master images
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "master", itemIds);

    // Find thumbnails for each master. There can be several thumbnails for one master image, get narrowest ones here.
    for (const auto masterId : itemIds)
    {
        // Thumbnail references ('thmb') are from the thumbnail image to the master image
        ImageFileReaderInterface::IdVector thumbIds;
        reader.getReferencedToItemListByType(contextId, masterId, "thmb", thumbIds);

        const int thumbId = *min_element(thumbIds.cbegin(), thumbIds.cend(),
            [&](uint32_t a, uint32_t b){ return (reader.getWidth(contextId, a) < reader.getWidth(contextId, b)); } );
        imageMap[masterId] = thumbId;
    }

    std::cout << "example3 ended\n";
    // We have now item IDs of thumbnails and master images. Decode and show them from imageMap as wanted.
}

/// Access and read derived images
void example4(const std::string testFile)
{
    std::cout << "example4 started\n";

    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize(testFile);
    const auto& properties = reader.getFileProperties();

    // Verify that the file has one or several images in the MetaBox
    if (not (properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) ||
        properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection)))
    {
        return;
    }    

    // Find item IDs of 'iden' (identity transformation) type derived images
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "iden", itemIds);

    const uint32_t itemId = itemIds.at(0); // For demo purposes, assume there was one 'iden' item

    // 'dimg' item reference points from the 'iden' derived item to the input(s) of the derivation
    reader.getReferencedFromItemListByType(contextId, itemId, "dimg", itemIds);
    const uint32_t sourceItemId = itemIds.at(0); // For demo purposes, assume there was one

    // Get 'iden' item properties to find out what kind of derivation it is
    const auto itemProperties = reader.getItemProperties(contextId, itemId);

    unsigned int rotation = 0;
    for (const auto& property : itemProperties)
    {
        // For example, handle 'irot' transformational property is anti-clockwise rotation
        if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT)
        {
            // Get property struct by index to access rotation angle
            rotation = reader.getPropertyIrot(contextId, property.index).rotation;
            break; // Assume only one property
        }
    }

    cout << "To render derived image item ID " << itemId << ":" << endl;
    cout << "-retrieve data for source image item ID " << sourceItemId << endl;
    cout << "-rotating image " << rotation << " degrees." << endl;

    std::cout << "example4 ended\n";
}

/// Access and read media track samples, thumbnail track samples and timestamps
void example5(const std::string testFile) {
    std::cout << "example5 started\n";

    HevcImageFileReader reader;

    reader.initialize(testFile);
    const auto& properties = reader.getFileProperties();

    // Print information for every track read
    for (const auto& trackProperties : properties.trackProperties) {
        const uint32_t contextId = trackProperties.first;
        cout << "Track ID " << contextId << endl; // Context ID corresponds to the track ID

        if (trackProperties.second.trackFeature.hasFeature(ImageFileReaderInterface::TrackFeature::IsMasterImageSequence)) {
            cout << "This is a master image sequence." << endl;
        }

        if (trackProperties.second.trackFeature.hasFeature(ImageFileReaderInterface::TrackFeature::IsThumbnailImageSequence)) {
            // Assume there is only one type track reference, so check reference type and master track ID(s) from the first one.
            const auto tref = trackProperties.second.referenceTrackIds.cbegin();
            cout << "Track reference type is '"<< tref->first << "'" << endl;
            cout << "This is a thumbnail track for track ID ";
            for (const auto masterTrackId : tref->second) {
                cout << masterTrackId << endl;
            }
        }

        ImageFileReaderInterface::TimestampMap timestamps;
        reader.getItemTimestamps(contextId, timestamps);
        cout << "Sample timestamps:" << endl;
        for (const auto& timestamp : timestamps) {
            cout << " Timestamp=" << timestamp.first << "ms, sample ID=" << timestamp.second << endl;
        }

        for (const auto& sampleProperties : trackProperties.second.sampleProperties) {
            ImageFileReaderInterface::IdVector itemsToDecode;
            const uint32_t sampleId = sampleProperties.first;

            // A sample might have decoding dependencies. The simples way to handle this is just to always ask and
            // decode all dependencies.
            reader.getItemDecodeDependencies(contextId, sampleId, itemsToDecode);
            for (auto dependencyId : itemsToDecode) {
                ImageFileReaderInterface::DataVector data;
                reader.getItemDataWithDecoderParameters(contextId, dependencyId, data);
                std::cout << data.size() << endl;
            }
            // Store or show the image...
        }
    }

    std::cout << "example5 ended\n";
}

/// Access and read media alternative
void example6(const std::string testFile)
{
    std::cout << "example6 started\n";

    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize(testFile);
    const auto& properties = reader.getFileProperties();

    uint32_t trackId = 0;

    if (properties.trackProperties.size() > 0)
    {
        auto iter = properties.trackProperties.cbegin(); // Get first image sequence track
        trackId = iter->first;
        const HevcImageFileReader::TrackProperties& trackProperties = iter->second;

        if (trackProperties.trackFeature.hasFeature(HevcImageFileReader::TrackFeature::HasAlternatives))
        {
            const uint32_t alternativeTrackId = trackProperties.alternateTrackIds.at(0); // Take the first alternative
            const auto alternativeWidth = reader.getDisplayWidth(alternativeTrackId);
            const auto trackWidth = reader.getDisplayWidth(trackId);

            if (trackWidth > alternativeWidth)
            {
                cout << "The alternative track has wider display width, let's use it from now on..." << endl;
                trackId = alternativeTrackId;
            }
        }
    }
    std::cout << "example6 ended\n";
}

int main() {
    const std::string testFile = "C031.heic";
    // example1(testFile);
    // example2(testFile);
    // example3(testFile);
    // example4(testFile);
    // example5(testFile);
    // example6(testFile);
    return extractMaster();
}

