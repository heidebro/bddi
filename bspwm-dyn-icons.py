#!/bin/python3
import socket
import os
import json
import collections

"""
Dict holding icon mappings for programs
"""
icon_dict = {
                "firefox": "",
                "st-256color": "",
                "mpv": "辶",
                "chromium": "",
                "thunderbird": "",
                "pcmanfm": "",
                "pavucontrol": "墳",
            }


def subscribe():
    """
    Subscribe to node changes in bspwm via its UNIX-Socket.
    Triggering appropriate desktop-name updates on changing nodes
    """
    client = get_client()
    client.send(b"subscribe\x00node\x00")
    
    data = client.recv(1024)
    while data:
        line = data.decode().replace("\n", " ").strip()
        fields = line.split(" ")

        if fields[0] == "node_add":
            desktop_id = fields[2]
            update_desktop(desktop_id)

        elif fields[0] == "node_transfer":
            src_desktop_id = fields[2]
            dest_desktop_id = fields[5]
            update_desktop(src_desktop_id)
            update_desktop(dest_desktop_id)

        elif fields[0] == "node_remove":
            desktop_id = fields[2]
            update_desktop(desktop_id)

        data = client.recv(1024)

def get_client():
    """
    Create, connect and return connection to bspwms UNIX-Socket 
    """
    client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    client.connect("/tmp/bspwm_0_0-socket")

    return client

def update_desktop(desktop_id):
    """
    Routine for updating a desktop
    """
    desktop_info = get_desktop_info(desktop_id)
    nodes = get_nodes_on_desktop(desktop_info["root"])
    name = get_desktop_name(nodes)
    set_desktop_name(desktop_id, name)

def get_desktop_info(desktop_id):
    """
    Retrieve JSON formatted desktop tree.
    """
    client = get_client()
    query = ("query\x00-T\x00-d\x00" + desktop_id + "\x00").encode("ascii")
    client.send(query)

    
    desktop_info = ""
    data = client.recv(512)

    while data: 
        desktop_info += data.decode()
        data = client.recv(512)

    desktop_info = json.loads(desktop_info)

    client.close()
    return desktop_info


def get_nodes_on_desktop(desktop_info):
    """
    Recursively traverse bspwm desktop tree, retrieving and returning all leaf nodes.
    """
    if not desktop_info:
        return None

    first = desktop_info["firstChild"]
    second = desktop_info["secondChild"]
    node = desktop_info["client"]

    if node:
        return [node]
    else:
        return get_nodes_on_desktop(first) + get_nodes_on_desktop(second)

def get_desktop_name(nodes):
    """
    Generate bspwm desktop name containing icons of all open nodes.
    """
    if not nodes:
        return "-"

    name = ""
    for node in nodes: 
        if not name:
            name = get_icon(node["className"])
        else:
            name += " " + get_icon(node["className"])

    return name

def get_icon(class_name):
    """
    Returns icon corresponding with given class_name if present in icon_dict.
    If class_name can not be found in icon_dict, a default icon is returned.
    """
    class_name = class_name.lower()
    
    try:
        return icon_dict[class_name]
    except:
        return ""

def set_desktop_name(desktop_id, name):
    """
    Rename bspwm desktop identified by desktop_id to name.
    """
    client = get_client()
    query = ("desktop\x00" + desktop_id + "\x00-n\x00" + name + "\x00").encode("utf-8")
    client.send(query)
    client.close()

# Start listening for events
subscribe()
