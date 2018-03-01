#!/bin/bash

set -x

# Cause the script to exit if a single command fails.
set -e

TP_DIR=$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)

CATAPULT_COMMIT=18cd334755701cf0c3b90b7172126c686d2eb787
CATAPULT_HOME=$TP_DIR/catapult
VULCANIZE_BIN=$CATAPULT_HOME/tracing/bin/vulcanize_trace_viewer

CATAPULT_FILES=$TP_DIR/../../python/ray/core/src/catapult_files

# This is where we will copy the files that need to be packaged with the wheels.
mkdir -p $CATAPULT_FILES

if [[ "$INCLUDE_UI" == "0" ]]; then
  # Let installation continue without building the UI.
  exit 0
fi

if ! type python2 > /dev/null; then
  echo "cannot properly set up UI without a python2 executable"
  if [[ "$INCLUDE_UI" == "1" ]]; then
    # Since the UI is explicitly supposed to be included, fail here.
    exit 1
  else
    # Let installation continue without building the UI.
    exit 0
  fi
fi

# Download catapult and use it to autogenerate some static html if it isn't
# already present.
if [[ ! -d $CATAPULT_HOME ]]; then
  echo "setting up catapult"
  # The git clone command seems to fail in Travis, so retry up to 20 times.
  for COUNT in {1..20}; do
    # Attempt to git clone catapult and break from the retry loop if it succeeds.
    git clone https://github.com/ray-project/catapult.git $CATAPULT_HOME && break
    # If none of the retries succeeded at getting boost, then fail.
    if [[ $COUNT == 20 ]]; then
      exit 1
    fi
  done

  # Check out the appropriate commit from catapult.
  pushd $CATAPULT_HOME
    git checkout $CATAPULT_COMMIT
  popd
fi

# If the autogenerated catapult files aren't present, then generate them.
if [[ ! -f $CATAPULT_FILES/index.html ]]; then
  python2 $VULCANIZE_BIN --config chrome --output $CATAPULT_FILES/trace_viewer_full.html
  cp $CATAPULT_HOME/tracing/bin/index.html $CATAPULT_FILES/index.html
fi
