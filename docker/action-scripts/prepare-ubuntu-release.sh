#!/bin/bash

chmod +x docker/building_dependencies/install_packages/debian.sh && ./docker/building_dependencies/install_packages/debian.sh
chmod +x docker/building_dependencies/clean_release.sh && ./docker/building_dependencies/clean_release.sh

mkdir $ARTIFACT_DIR
chmod 777 $ARTIFACT_DIR

UTBOT_DISTR_FOLDER=$ARTIFACT_DIR/utbot_distr
if [ -d "$UTBOT_DISTR_FOLDER" ]; then rm -rf $UTBOT_DISTR_FOLDER; fi

cp -r /utbot_distr $UTBOT_DISTR_FOLDER
cp -a docker/release_distribution_scripts/. $UTBOT_DISTR_FOLDER/
echo $VERSION > $ARTIFACT_DIR/version.txt

mv vscode-plugin/*.vsix $ARTIFACT_DIR/utbot_plugin.vsix
mv docker/unpack_and_run_utbot.sh $ARTIFACT_DIR/unpack_and_run_utbot.sh

cd $ARTIFACT_DIR
tar -czf utbot_distr.tar.gz utbot_distr
rm -rf utbot_distr
