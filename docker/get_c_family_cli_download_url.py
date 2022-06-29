import json
import requests

C_FAMILY_ARTIFACTS_URL="https://api.github.com/repos/UnitTestBot/UTBotCPP/actions/artifacts"

request = requests.get(url = C_FAMILY_ARTIFACTS_URL)
data = request.json()
artifacts = data['artifacts']

for artifact in artifacts:
    if "utbot-dev" in artifact['name']:
        print(artifact['archive_download_url'])
        break
