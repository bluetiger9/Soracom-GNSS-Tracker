import logging
import json
import boto3

client = boto3.client('iot-data')
iot_client = boto3.client('iot')

# Setup logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

def lambda_handler(event, context):
    print(event)
    
    # Parse the JSON message 
    #jsonObj = json.loads(event)
    jsonObj = event
    
    imsi = jsonObj['imsi']
    payload = jsonObj['payloads']
    
    print("IMSI: ")
    print(imsi)
    print("Payload: ")
    print(payload)
  
    # iot thing lookup
    thingsResponse = iot_client.list_things(
        maxResults = 1,
        attributeName = "soracom_imsi",
        attributeValue = imsi
        #thingTypeName = '3d-printer'
    )
    
    if len(thingsResponse['things']) == 1:
        # thing found
        thingName = thingsResponse['things'][0]['thingName']
        print("Thing found: " + thingName)
            
    else:
        # thing not found
        print("Thing not found")
        return {
            'statusCode': 200,
            'body': json.dumps('Thing not found!')
        }
        
    # update shadow
    response = client.update_thing_shadow(
        thingName = thingName,
        payload = json.dumps({
            'state': {
                'reported': payload
            }
        }
        )
    )
    
    return {
        'statusCode': 200,
        'body': json.dumps('Hello from Lambda!')
    }
