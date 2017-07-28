# -*- coding: utf-8 -*- 
from os import walk
import os
import json
import httplib
import hashlib
import urllib
import sys
import time

PHOTOSBUILDER_HOST = "localhost"
PHOTOSBUILDER_PORT = 8095
B2_HOST = "10.0.55.121"
B2_PORT = "80"
B2_API_KEY = "n6dl64BsbmCpzxyL7gnOy5gq8ZhDCH6xDBLtXdqLNoM824V"
TEST_USERNAME = "roland"
PHOTOSTORE_HOST = "localhost"
PHOTOSTORE_PORT = 8097
VERSION_PS = "v1"

def main():

    heic_files = get_heic_files()
    if heic_files is None:
        return

    if not get_file_sizes(heic_files):
        return

    if not generate_md5(heic_files):
        return
    
    if not get_download_hashes(heic_files):
        return

    add_remaining_vars(heic_files)

    '''if not post_heic_files_to_builder(heic_files):
        return'''

    if not generate_photo_ids(heic_files):
        return

    if not generate_html(heic_files):
        return

    if not generate_curls(heic_files):
        return

    print "heic files processed!"

def get_heic_files():
    heic_files = []
    for (_, _, filenames) in walk("."):
        for filename in filenames:
            if ".HEIC" in filename.upper():
                heic_file = {"filename": filename, "username": TEST_USERNAME}
                heic_files.append(heic_file)

    return heic_files

def get_file_sizes(heic_files):
    for heic_file in heic_files:
        try:
            heic_file["filesize"] = os.path.getsize(heic_file["filename"])
        except Exception as e:
            print "unable to get file size, reason: %s" % str(e)
            return False
    return True

def generate_md5(heic_files):
    for heic_file in heic_files:
        hash_md5 = hashlib.md5()
        with open(heic_file["filename"], "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        heic_file["md5"] = hash_md5.hexdigest()
    return True


def get_download_hashes(heic_files):
    for heic_file in heic_files:
        if not get_download_hash(heic_file):
            return False
    return True

def get_download_hash(heic_file):
    try:
        conn = httplib.HTTPConnection(B2_HOST, B2_PORT)
        conn.request("GET", "/apin/admin/getDownloadHash/%s/%s?APIKEY=%s" % (heic_file["username"], heic_file["md5"], B2_API_KEY))
        response = conn.getresponse()
    except Exception as e:
        print "error sending request to b2, reason %s" % str(e)
        return False

    if response.status != 200:
        conn.close()
        print "bad response code %d from b2" % response
        return False

    #check if builder responds correctly
    response_json = json.load(response)
    if not "downloadHash" in response_json:
        conn.close()
        print "bad json response"
        return False

    conn.close()
    heic_file["location"] = response_json["downloadHash"]    
    return True


def add_remaining_vars(heic_files):
    for heic_file in heic_files:
        heic_file["event_timestamp"] = int(round(time.time() * 1000))
        heic_file["modified"] = "%d" % int(round(time.time() * 1000))
        heic_file["mimetype"] = "image/heif"
        heic_file["source"] = "full_import"
        heic_file["created"] = "%d" % int(round(time.time() * 1000))
        heic_file["is_last"] = False
        heic_file["overwrite"] = False
        heic_file["path"] = "/markoselin1/iPhone/STC_0057.JPG"
        heic_file["x-id"] = "PQM-bdxMEytA8pz4"

def post_heic_files_to_builder(heic_files):
    for heic_file in heic_files:
        if not post_heic_file_to_builder(heic_file):
            #return False
            print "skipping %s" % heic_file["filename"]
    return True

def post_heic_file_to_builder(heic_file):
    try:
        conn = httplib.HTTPConnection(PHOTOSBUILDER_HOST, PHOTOSBUILDER_PORT)
        entry_json = json.dumps(heic_file)
        params = {"fileJson": entry_json}
        params_encoded = urllib.urlencode(params)
        headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
        conn.request("POST", "/upload", params_encoded, headers)
        response = conn.getresponse()
    except Exception as e:
        print "error sending request to photosbuilder for id: %s, reason %s" % (heic_file["filename"], str(e))
        return False

    if response.status != 200:
        conn.close()
        print "bad response code %d from photosbuilder for id %s" % (response.status, heic_file["filename"])
        return False

    conn.close()
    return True

def generate_photo_ids(heic_files):
    for heic_file in heic_files:
        if not generate_photo_id(heic_file):
            return False
    return True

def generate_photo_id(heic_file):
    try:
        id_string = heic_file["username"]+heic_file["md5"]
        id_string_bin = id_string.encode()
        hash_object = hashlib.sha1(id_string_bin)
        hex_dig = hash_object.hexdigest()
        heic_file["ps_id"] = hex_dig
    except Exception as e:
        print "could not generate photos id, reason: %s" % str(e)
        return False
    return True

def generate_html(heic_files):
    try:
        html_file = open("thumbs.htm", "w")
    except Exception as e:
        print "unable to open thumbs.htm, reason: %s" % str(e)
        return False

    for heic_file in heic_files:
        small = "<img src=http://%s:%d/thumbs_no_jwt/%s/%s.s><br>\n" % (PHOTOSTORE_HOST, PHOTOSTORE_PORT, VERSION_PS, heic_file["ps_id"])
        medium = "<img src=http://%s:%d/thumbs_no_jwt/%s/%s.m><br>\n" % (PHOTOSTORE_HOST, PHOTOSTORE_PORT, VERSION_PS, heic_file["ps_id"])
        large = "<img src=http://%s:%d/thumbs_no_jwt/%s/%s.l><br>\n" % (PHOTOSTORE_HOST, PHOTOSTORE_PORT, VERSION_PS, heic_file["ps_id"])
        html_file.write("<h1>" + heic_file["filename"] + "</h1>" + "<br>" + small + "<b>" + medium + "<br>" + large)

    html_file.close()
    return True

def generate_curls(heic_files):
    try: 
        text_file = open("curls.txt", "w")
    except Exception as e:
        print "unable to open curls dump file, reason: %s" % str(e)
        return False

    for heic_file in heic_files:
        text_file.write("curl -X GET -o \"%s.jpg\" http://%s:%d/thumbs_no_jwt/%s/%s.l\n" % (heic_file["filename"], PHOTOSTORE_HOST, PHOTOSTORE_PORT, VERSION_PS, heic_file["ps_id"]))

    text_file.close()
    return True

if __name__ == "__main__":
    main()