#!/bin/bash -e

VERSION=$1

if [[ -z "${VERSION}" ]]; then
    echo "ERROR: no version to tag given"
    exit 1
fi

if [[ -n "$(git tag | grep "${VERSION}" || true)" ]]; then
    echo "ERROR: tag name already used"
    exit 1
fi

sed -i "s/\"version\":.*/\"version\": \"${VERSION}\"/" package.json
git commit package.json -m "Updated version in package.json to ${VERSION}"
git tag -a "${VERSION}" -m "Created tag ${VERSION}"

echo "INFO: If everything looks good push the release with the following command, so the firmware will be build"
echo "CMD: git push -u origin main --tags"
