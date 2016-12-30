#! /bin/bash
docker build -t simonferquel/moby-remote:linux-${BUILD_SOURCEVERSION} .
docker push simonferquel/moby-remote:linux-${BUILD_SOURCEVERSION}