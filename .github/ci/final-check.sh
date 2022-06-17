#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
echo $STEPS_CONTEXT
step=$1
status=$2
export BODYMESSAGE="$(git log -1 $GITHUB_SHA --pretty=oneline --abbrev-commit)"
export BACKTICK='`';
export TIMESTAMP=$(date --utc +%FT%TZ);
export GITHUB_ACTOR_NAME="$(git log -1 $GITHUB_SHA --pretty="%aN")";
export COMMIT_FORMATTED="[$BACKTICK${GITHUB_SHA:0:7}$BACKTICK](https://github.com/$GITHUB_REPOSITORY/commit/$GITHUB_SHA)";

if [[ "$status" == "success" ]];
then
    echo "Success build"
    curl -v -H User-Agent:bot -H Content-Type:application/json -d '{"avatar_url":"https://pngimg.com/uploads/github/github_PNG90.png","username":"github-action","embeds":[{"author":{"name":"Build #'"$step"' Passed - '"$GITHUB_ACTOR_NAME"'","url":"https://github.com/'"$GITHUB_REPOSITORY"'/actions/runs/'"$GITHUB_RUN_ID"'"},"url":"https://github.com/'"$GITHUB_REPOSITORY"'/actions/runs/'"$GITHUB_RUN_ID"'","title":"['"$GITHUB_REPOSITORY"':job#'"$GITHUB_RUN_NUMBER"'] ","color":65280,"fields":[{"name":"_ _", "value": "'"$COMMIT_FORMATTED"' - '"$BODYMESSAGE"'"}],"timestamp":"'"$TIMESTAMP"'","footer":{"text":"ESP3D CI"}}]}' $DISCORD_WEBHOOK_URL;
else
    echo "Build failed"
    curl -v -H User-Agent:bot -H Content-Type:application/json -d '{"avatar_url":"https://pngimg.com/uploads/github/github_PNG90.png","username":"github-action","embeds":[{"author":{"name":"Build #'"$step"' Failed - '"$GITHUB_ACTOR_NAME"'","url":"https://github.com/'"$GITHUB_REPOSITORY"'/actions/runs/'"$GITHUB_RUN_ID"'"},"url":"https://github.com/'"$GITHUB_REPOSITORY"'/actions/runs/'"$GITHUB_RUN_ID"'","title":"['"$GITHUB_REPOSITORY"':job#'"$GITHUB_RUN_NUMBER"'] ","color":16711680,"fields":[{"name":"_ _", "value": "'"$COMMIT_FORMATTED"' - '"$BODYMESSAGE"'"}],"timestamp":"'"$TIMESTAMP"'","footer":{"text":"ESP3D CI"}}]}' $DISCORD_WEBHOOK_URL;
    exit 1
fi


