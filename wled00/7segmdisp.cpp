#include "7segmdisp.h"

static CRGB dummy;

void LedBasedDisplay::setRowColor(uint16_t row, bool state, CRGB color) {
    for (uint16_t c = 0, n = columnCount(); c < n; ++c) {
        setLedColor(row, c, state, color);
    }
}

void LedBasedDisplay::setColumnColor(uint16_t column, bool state, CRGB color) {
    for (uint16_t r = 0, n = rowCount(); r < n; ++r) {
        setLedColor(r, column, state, color);
    }
}

void LedBasedDisplay::setColor(bool state, CRGB color) {
    for (uint16_t r = 0, rn = rowCount(); r < rn; ++r) {
        for (uint16_t c = 0, cn = columnCount(); c < cn; ++c) {
            setLedColor(r, c, state, color);
        }
    }
}

SevenSegmentDisplay::SevenSegmentDisplay(LedBasedDisplayOutput output, uint16_t ledsPerSegment):
    _output(output),
    _backgroundColor(CRGB::Black),
    _ledsPerSegment(ledsPerSegment),
    _value(_7SEG_SYM_EMPTY),
    _paintBackground(false),
    _mode(LedBasedDisplayMode::SET_ALL_LEDS) {

    _offColors = (CRGB*) malloc(7 * _ledsPerSegment * sizeof(CRGB));
    _onColors = (CRGB*) malloc(7 * _ledsPerSegment * sizeof(CRGB));

    _showZero = true;
}

SevenSegmentDisplay::~SevenSegmentDisplay() {
    delete _offColors;
    delete _onColors;
}

uint16_t SevenSegmentDisplay::rowCount() {
    return _ledsPerSegment * 2 + 3;
}

uint16_t SevenSegmentDisplay::columnCount() {
    return _ledsPerSegment + 2;
}

CRGB SevenSegmentDisplay::getBackgroundColor() {
    return _backgroundColor;
}

void SevenSegmentDisplay::setBackgroundColor(CRGB color) {
    _backgroundColor = color;
}

CRGB* SevenSegmentDisplay::getLedColor(uint16_t row, uint16_t column, bool state) {
    uint16_t idx = _internalIndex(segmentOfCoords(row, column), row, column);

    if (idx == _7SEG_UNDEF) {
        return &dummy;
    }

    return &(state ? _onColors : _offColors)[idx];
}

void SevenSegmentDisplay::setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) {
    uint16_t idx = _internalIndex(segmentOfCoords(row, column), row, column);

    if (idx == _7SEG_UNDEF) {
        return;
    }

    (state ? _onColors : _offColors)[idx] = color;
}

void SevenSegmentDisplay::update(uint16_t rowOffset, uint16_t columnOffset, uint16_t width, uint16_t height) {
    for (uint16_t r = 0; r < height; ++r) {
        for (uint16_t c = 0; c < width; ++c) {
            uint16_t segment = segmentOfCoords(r, c);
            if (segment != _7SEG_UNDEF) {
                bool on = _value & _7SEG_MASK(segment);
                if (_7SEG_MODE(_mode, on)) {
                    CRGB crgb = (on ? _onColors : _offColors)[_internalIndex(segment, r, c)];
                    (*_output)(c + columnOffset, r + rowOffset, crgb.red, crgb.green, crgb.blue);
                }
            } else if (_paintBackground) {
                (*_output)(c + columnOffset, r + rowOffset, _backgroundColor.red, _backgroundColor.green, _backgroundColor.blue);
            }
        }
    }
}

void SevenSegmentDisplay::setPaintBackground(bool paint) {
    _paintBackground = paint;
}

void SevenSegmentDisplay::setMode(LedBasedDisplayMode mode) {
    _mode = mode;
}

uint16_t SevenSegmentDisplay::segmentOfCoords(uint16_t row, uint16_t column) {
    uint16_t midRow = _ledsPerSegment + 1;
    uint16_t lastRow = midRow * 2;

    if (row < 0 || row > lastRow) {
        return _7SEG_UNDEF;
    }

    bool top = row == 0;
    bool mid = row == midRow;
    bool bot = row == lastRow;

    if (top || mid || bot) {
        if (column >= 1 && column <= _ledsPerSegment) {
            if (top) {
                return _7SEG_SEG_A;
            } else if (mid) {
                return _7SEG_SEG_G;
            } else {
                return _7SEG_SEG_D;
            }
        }
    } else if (column == 0) {
        if (row < midRow) {
            return _7SEG_SEG_F;
        } else {
            return _7SEG_SEG_E;
        }
    } else if (column == _ledsPerSegment + 1) {
        if (row < midRow) {
            return _7SEG_SEG_B;
        } else {
            return _7SEG_SEG_C;
        }
    }

    return _7SEG_UNDEF;
}

void SevenSegmentDisplay::setSymbol(uint16_t symbol) {
    _value = symbol;
}

void SevenSegmentDisplay::setDigit(uint16_t digit) {
    switch (digit) {
        case 0:
            if (_showZero)
                setSymbol(_7SEG_SYM_0);
            else
                setSymbol(_7SEG_SYM_EMPTY);
            break;
        case 1: setSymbol(_7SEG_SYM_1); break;
        case 2: setSymbol(_7SEG_SYM_2); break;
        case 3: setSymbol(_7SEG_SYM_3); break;
        case 4: setSymbol(_7SEG_SYM_4); break;
        case 5: setSymbol(_7SEG_SYM_5); break;
        case 6: setSymbol(_7SEG_SYM_6); break;
        case 7: setSymbol(_7SEG_SYM_7); break;
        case 8: setSymbol(_7SEG_SYM_8); break;
        case 9: setSymbol(_7SEG_SYM_9); break;
        default: setSymbol(_7SEG_SYM_EMPTY);
    }
}

void SevenSegmentDisplay::setCharacter(char charcter) {
    switch (charcter) {
        case 'a': case 'A': setSymbol(_7SEG_SYM_A); break;
        case 'b': case 'B': setSymbol(_7SEG_SYM_B); break;
        case 'c': case 'C': setSymbol(_7SEG_SYM_C); break;
        case 'd': case 'D': setSymbol(_7SEG_SYM_D); break;
        case 'e': case 'E': setSymbol(_7SEG_SYM_E); break;
        case 'f': case 'F': setSymbol(_7SEG_SYM_F); break;
        case 'g': case 'G': setSymbol(_7SEG_SYM_G); break;
        case 'h': case 'H': setSymbol(_7SEG_SYM_H); break;
        case 'i': case 'I': setSymbol(_7SEG_SYM_I); break;
        case 'j': case 'J': setSymbol(_7SEG_SYM_J); break;
        case 'k': case 'K': setSymbol(_7SEG_SYM_K); break;
        case 'l': case 'L': setSymbol(_7SEG_SYM_L); break;
        case 'm': case 'M': setSymbol(_7SEG_SYM_M); break;
        case 'n': case 'N': setSymbol(_7SEG_SYM_N); break;
        case 'o': case 'O': setSymbol(_7SEG_SYM_O); break;
        case 'p': case 'P': setSymbol(_7SEG_SYM_P); break;
        case 'q': case 'Q': setSymbol(_7SEG_SYM_Q); break;
        case 'r': case 'R': setSymbol(_7SEG_SYM_R); break;
        case 's': case 'S': setSymbol(_7SEG_SYM_S); break;
        case 't': case 'T': setSymbol(_7SEG_SYM_T); break;
        case 'u': case 'U': setSymbol(_7SEG_SYM_U); break;
        case 'v': case 'V': setSymbol(_7SEG_SYM_V); break;
        case 'w': case 'W': setSymbol(_7SEG_SYM_W); break;
        case 'x': case 'X': setSymbol(_7SEG_SYM_X); break;
        case 'y': case 'Y': setSymbol(_7SEG_SYM_Y); break;
        case 'z': case 'Z': setSymbol(_7SEG_SYM_Z); break;
        case '-': setSymbol(_7SEG_SYM_DASH); break;
        case '_': setSymbol(_7SEG_SYM_UNDERSCORE); break;
        case ' ':
        default: setSymbol(_7SEG_SYM_EMPTY);
    }
}

void SevenSegmentDisplay::setShowZero(bool showZero) {
    _showZero = showZero;
}

uint16_t SevenSegmentDisplay::_internalIndex(uint16_t segment, uint16_t row, uint16_t column) {
    switch (segment) {
    case _7SEG_SEG_A: return _7SEG_IDX(_7SEG_SEG_A, column - 1);
    case _7SEG_SEG_B: return _7SEG_IDX(_7SEG_SEG_B, row - 1);
    case _7SEG_SEG_C: return _7SEG_IDX(_7SEG_SEG_C, row - _ledsPerSegment - 2);
    case _7SEG_SEG_D: return _7SEG_IDX(_7SEG_SEG_D, column - 1);
    case _7SEG_SEG_E: return _7SEG_IDX(_7SEG_SEG_E, row - _ledsPerSegment - 2);
    case _7SEG_SEG_F: return _7SEG_IDX(_7SEG_SEG_F, row - 1);
    case _7SEG_SEG_G: return _7SEG_IDX(_7SEG_SEG_G, column - 1);
    default: return _7SEG_UNDEF;
    }
}

SeparatorDisplay::SeparatorDisplay(LedBasedDisplayOutput output, uint16_t ledCount):
    _output(output),
    _maxLeds(ledCount),
    _ledCount(0),
    _mappings(NULL),
    _backgroundColor(CRGB::Black),
    _paintBackground(false),
    _state(false),
    _mode(LedBasedDisplayMode::SET_ALL_LEDS) {

        _mappings = (_Mapping*) malloc(sizeof(_Mapping) * ledCount);
    }

SeparatorDisplay::~SeparatorDisplay() {
    delete _mappings;
}

uint16_t SeparatorDisplay::rowCount() {
    uint16_t max = _mappings[0].row;
    for (uint16_t i = 1; i < _ledCount; ++i) {
        if (_mappings[i].row > max) {
            max = _mappings[i].row;
        }
    }
    return max + 1;
}

uint16_t SeparatorDisplay::columnCount() {
    uint16_t max = _mappings[0].column;
    for (uint16_t i = 1; i < _ledCount; ++i) {
        if (_mappings[i].column > max) {
            max = _mappings[i].column;
        }
    }
    return max + 1;
}

CRGB SeparatorDisplay::getBackgroundColor() {
    return _backgroundColor;
}

void SeparatorDisplay::setBackgroundColor(CRGB color) {
    _backgroundColor = color;
}

CRGB* SeparatorDisplay::getLedColor(uint16_t row, uint16_t column, bool state) {
    for (uint16_t i = 0; i < _ledCount; ++i) {
        if (_mappings[i].row == row && _mappings[i].column == column) {
            if (state) {
                return &_mappings[i].onColor;
            } else {
                return &_mappings[i].offColor;
            }
        }
    }

    return &dummy;
}

void SeparatorDisplay::setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) {
    for (uint16_t i = 0; i < _ledCount; ++i) {
        if (_mappings[i].row == row && _mappings[i].column == column) {
            if (state) {
                _mappings[i].onColor = color;
            } else {
                _mappings[i].offColor = color;
            }
        }
    }
}

void SeparatorDisplay::update(uint16_t rowOffset, uint16_t columnOffset, uint16_t width, uint16_t height) {
    for (uint16_t r = 0; r < height; ++r) {
        for (uint16_t c = 0; c < width; ++c) {
            bool found = false;
            for (uint16_t i = 0; i < _ledCount; ++i) {
                if (_mappings[i].row == r && _mappings[i].column == c) {
                    if (_7SEG_MODE(_mode, _state)) {
                        CRGB crgb = _state ? _mappings[i].onColor : _mappings[i].offColor;
                        (*_output)(c + columnOffset, r + rowOffset, crgb.red, crgb.green, crgb.blue);
                    }
                    found = true;
                    break;
                }
            }
            if (!found && _paintBackground) {
                (*_output)(c + columnOffset, r + rowOffset, _backgroundColor.red, _backgroundColor.green, _backgroundColor.blue);
            }
        }
    }
}

void SeparatorDisplay::setPaintBackground(bool paint) {
    _paintBackground = paint;
}

void SeparatorDisplay::setMode(LedBasedDisplayMode mode) {
    _mode = mode;
}

void SeparatorDisplay::addLed(uint16_t row, uint16_t column) {
    _mappings[_ledCount].row = row;
    _mappings[_ledCount].column = column;
    _ledCount++;
}

void SeparatorDisplay::setState(bool state) {
    _state = state;
}

LedBasedRowDisplay::LedBasedRowDisplay(uint16_t displayCount, ...):
    _displayCount(displayCount),
    _displays((LedBasedDisplay**) malloc(displayCount * sizeof(LedBasedDisplay*))) {

    va_list ap;
    va_start(ap, displayCount);

    for (uint16_t i = 0; i < displayCount; i++) {
        _displays[i] = va_arg(ap, LedBasedDisplay*);
    }

    va_end(ap);
}

LedBasedRowDisplay::~LedBasedRowDisplay() {
    delete _displays;
}

uint16_t LedBasedRowDisplay::rowCount() {
    uint16_t rowCount = 0;
    for (uint16_t i = 0; i < _displayCount; i++) {
        rowCount = max(rowCount, _displays[i]->rowCount());
    }
    return rowCount;
}

uint16_t LedBasedRowDisplay::columnCount() {
    uint16_t columnCount = 0;
    for (uint16_t i = 0; i < _displayCount; i++) {
        columnCount += _displays[i]->columnCount();
    }
    return columnCount;
}

CRGB LedBasedRowDisplay::getBackgroundColor() {
    if (_displayCount > 0) {
        return _displays[0]->getBackgroundColor();
    } else {
        return CRGB::Black;
    }
}

void LedBasedRowDisplay::setBackgroundColor(CRGB color) {
    for (uint16_t i = 0; i < _displayCount; i++) {
        _displays[i]->setBackgroundColor(color);
    }
}

CRGB* LedBasedRowDisplay::getLedColor(uint16_t row, uint16_t column, bool state) {
    uint16_t c = column;
    for (uint16_t i = 0; i < _displayCount; i++) {
        uint16_t cc = _displays[i]->columnCount();
        if (_displays[i]->columnCount() - 1 < c) {
            c -= cc;
        } else {
            return _displays[i]->getLedColor(row, c, state);
        }
    }
    return &dummy;
}

void LedBasedRowDisplay::setLedColor(uint16_t row, uint16_t column, bool state, CRGB color) {
    uint16_t c = column;
    for (uint16_t i = 0; i < _displayCount; i++) {
        uint16_t cc = _displays[i]->columnCount();
        if (cc - 1 < c) {
            c -= cc;
        } else {
            _displays[i]->setLedColor(row, c, state, color);
            return;
        }
    }
}

void LedBasedRowDisplay::update(uint16_t rowOffset, uint16_t columnOffset, uint16_t width, uint16_t height) {
    uint16_t h = rowCount();
    for (uint16_t i = 0; i < _displayCount; i++) {
        uint16_t cc = _displays[i]->columnCount();
        _displays[i]->update(rowOffset, columnOffset, cc, h);
        columnOffset += cc;
    }
}

void LedBasedRowDisplay::setPaintBackground(bool paint) {
    for (uint16_t i = 0; i < _displayCount; i++) {
        _displays[i]->setPaintBackground(paint);
    }
}

void LedBasedRowDisplay::setMode(LedBasedDisplayMode mode) {
    for (uint16_t i = 0; i < _displayCount; i++) {
        _displays[i]->setMode(mode);
    }
}
