#!/usr/bin/env bash
echo 'Sending Discord Webhook';
export BACKTICK='`';
export TIMESTAMP=$(date --utc +%FT%TZ);
export REPO_OWNER="luc-github";
export REPO_NAME="ESP3D";
export COMMIT_FORMATTED="[$BACKTICK${TRAVIS_COMMIT:0:7}$BACKTICK](https://github.com/$REPO_OWNER/$REPO_NAME/commit/$TRAVIS_COMMIT)";
curl -v -H User-Agent:bot -H Content-Type:application/json -d '{"avatar_url":"https://i.imgur.com/kOfUGNS.png","username":"Travis CI","embeds":[{"author":{"name":"Build #'"$TRAVIS_BUILD_NUMBER"' Failed - '"$AUTHOR_NAME"'","url":"https://travis-ci.org/'"$REPO_OWNER"'/'"$REPO_NAME"'/builds/'"$TRAVIS_BUILD_ID"'"},"url":"https://github.com/'"$REPO_OWNER"'/'"$REPO_NAME"'/commit/'"$TRAVIS_COMMIT"'","title":"['"$TRAVIS_REPO_SLUG"':'"$TRAVIS_BRANCH"'] ","color":16711680,"fields":[{"name":"_ _", "value": "'"$COMMIT_FORMATTED"' - '"$TRAVIS_COMMIT_MESSAGE"'"}],"timestamp":"'"$TIMESTAMP"'","footer":{"text":"Travis CI"}}]}' $DISCORD_WEBHOOK_URL;
