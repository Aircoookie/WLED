const assert = require('node:assert');
const { describe, it, before, after, mock, test } = require('node:test');
const fs = require('fs');
const path = require('path');
const cdata = require('./cdata.js');

describe('Caching', function () {
  describe('isFileNewerThan', function () {
    before(function () {
      // Create a temporary file before the test
      fs.writeFileSync('temp.txt', 'Hello World');
    });

    it('should return true if the file is newer than the provided time', function () {
      const pastTime = Date.now() - 10000; // 10 seconds ago
      assert.equal(cdata.isFileNewerThan('temp.txt', pastTime), true);
    });

    it('should return false if the file is older than the provided time', function () {
      const futureTime = Date.now() + 10000; // 10 seconds in the future
      assert.equal(cdata.isFileNewerThan('temp.txt', futureTime), false);
    });

    it('should return false if the file does not exist', function () {
      assert.equal(cdata.isFileNewerThan('nonexistent.txt', Date.now()), false);
    });

    // delete the temporary file after the test
    after(function () {
      fs.unlinkSync('temp.txt');
    });
  });

  describe('isAnyFileInFolderNewerThan', function () {
    const testFolderPath = path.join(__dirname, 'testFolder');
    const oldFilePath = path.join(testFolderPath, 'oldFile.txt');
    const newFilePath = path.join(testFolderPath, 'newFile.txt');

    before(function () {
      // Create test folder
      if (!fs.existsSync(testFolderPath)) {
        fs.mkdirSync(testFolderPath);
      }

      // Create an old file
      fs.writeFileSync(oldFilePath, 'This is an old file.');
      // Modify the 'mtime' to simulate an old file
      const oldTime = new Date();
      oldTime.setFullYear(oldTime.getFullYear() - 1);
      fs.utimesSync(oldFilePath, oldTime, oldTime);

      // Create a new file
      fs.writeFileSync(newFilePath, 'This is a new file.');
    });

    it('should return true if a file in the folder is newer than the given time', function () {
      const folderPath = path.join(__dirname, 'testFolder');
      const time = fs.statSync(path.join(folderPath, 'oldFile.txt')).mtime;
      assert.strictEqual(cdata.isAnyFileInFolderNewerThan(folderPath, time), true);
    });

    it('should return false if no files in the folder are newer than the given time', function () {
      const folderPath = path.join(__dirname, 'testFolder');
      const time = new Date();
      assert.strictEqual(cdata.isAnyFileInFolderNewerThan(folderPath, time), false);
    });

    after(function () {
      // Delete test folder
      if (fs.existsSync(testFolderPath)) {
        fs.rmSync(testFolderPath, { recursive: true });
      }
    });
  });
});