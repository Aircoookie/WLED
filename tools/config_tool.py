#!/usr/bin/env python3

import requests
import argparse
import json

class WLEDClient:
    connectionString = None
    currentState = {}
    currentPresets = [None] * 16

    def __init__(self, connectionString):
        self.connectionString = connectionString
        self.getState()

    def getState(self):
        wledRequest = requests.get("{0}/json/state".format(self.connectionString))
        wledRequest.raise_for_status()
        self.currentState = wledRequest.json()
        return self.currentState

    def postState(self):
        wledRequest = requests.post("{0}/json/state".format(self.connectionString), json=self.currentState)
        wledRequest.raise_for_status()

    def changeState(self, changes):
        self.currentState = {**self.currentState, **changes}
        return self.currentState

    def loadPreset(self, presetID):
        wledRequest = requests.post("{0}/json/state".format(self.connectionString), json={"ps": presetID})
        wledRequest.raise_for_status()
        self.getState()

    def getPresets(self):
        self.getState()
        for i in range(0,16):
            self.loadPreset(i + 1)
            self.currentPresets[i] = self.getState()
        return self.currentPresets

    def postPreset(self, presetID, state):
        self.loadPreset(presetID)
        state["psave"] = presetID
        wledRequest = requests.post("{0}/json/state".format(self.connectionString), json=state)
        wledRequest.raise_for_status()
        print("Saving preset {}".format(presetID))




def main(args):
    wledClient = WLEDClient(args.wled_host)
    if args.action == "save":
        with open(args.file, "w") as configFile:
            json.dump(wledClient.currentState, configFile)
        print(wledClient.currentState)
    elif args.action == "restore":
        with open(args.file, "r") as configFile:
            newState = json.load(configFile)
            wledClient.changeState(newState)
            wledClient.postState()
    elif args.action == "save_presets":
        with open(args.file, "w") as presetsFile:
            wledClient.getPresets()
            json.dump(wledClient.currentPresets, presetsFile)
        print(wledClient.currentPresets)
    elif args.action == "restore_presets":
        with open(args.file, "r") as presetsFile:
            newPresets = json.load(presetsFile)
            for i in range(0, len(newPresets)):
                print(newPresets[i])
                wledClient.postPreset(i + 1, newPresets[i])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Save and Restore Current State for WLED Devices")
    parser.add_argument("action", default="save",
                        help="Action to take", choices=["save","restore","save_presets","restore_presets"])
    parser.add_argument("wled_host",
                        help="Host and Protocol for JSON API (e.g. `http://172.16.3.42`)")
    parser.add_argument("file", help="File to save/restore from")
    args = parser.parse_args()
    main(args)
