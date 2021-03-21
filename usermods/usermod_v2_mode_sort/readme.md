# Mode Sort

v2 usermod that provides data about modes and
palettes to other usermods. Notably it provides:
* A direct method for a mode or palette name
* Ability to retrieve mode and palette names in 
  alphabetical order

```char **getModesQStrings()```

Provides an array of char* (pointers) to the names of the
palettes within JSON_mode_names, in the same order as 
JSON_mode_names. These strings end in double quote (")
(or \0 if there is a problem).

```byte *getModesAlphaIndexes()```

An array of byte designating the indexes of names of the
modes in alphabetical order. "Solid" will always remain 
at the front of the list.

```char **getPalettesQStrings()```

Provides an array of char* (pointers) to the names of the
palettes within JSON_palette_names, in the same order as 
JSON_palette_names. These strings end in double quote (")
(or \0 if there is a problem).

```byte *getPalettesAlphaIndexes()```

An array of byte designating the indexes of names of the
palettes in alphabetical order. "Default" and those
starting with "(" will always remain at the front of the list.
