module.exports = [
    {
      "type": "heading",
      "defaultValue": "Pebblemon Time Configuration"
    },
    {
      "type": "text",
      "defaultValue": "Make Pebblemon Time your own."
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Background Colors"
        },
        {
          "type": "color",
          "messageKey": "BackgroundTopColor",
          "defaultValue": "0xFF0000",
          "label": "Top",
          "sunlight": false,
          "allowGray": true
        },
        {
          "type": "color",
          "messageKey": "BackgroundBottomColor",
          "defaultValue": "0xFFFFFF",
          "label": "Bottom",
          "sunlight": false,
          "allowGray": true
        },
        {
          "type": "color",
          "messageKey": "MiddleBarColor",
          "defaultValue": "0x000000",
          "label": "Middle Bar Color",
          "sunlight": false,
          "allowGray": true,
          "capabilities": ["ROUND"],
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Time Settings"
        },
        {
          "type": "radiogroup",
          "messageKey": "TimeStyle",
          "label": "Time Style",
          "defaultValue": "analog",
          "options": [
              {
                  "label": "Analog",
                  "value": "analog"
              },
              {
                  "label": "Digital",
                  "value": "digital"
              }
          ]
        },
        {
          "type": "color",
          "messageKey": "HourColor",
          "defaultValue": "0xFF0000",
          "label": "Hour Color",
          "sunlight": false,
          "allowGray": false
        },
        {
          "type": "color",
          "messageKey": "HourOutlineColor",
          "defaultValue": "0xFFFFFF",
          "label": "Hour Outline Color",
          "sunlight": false,
          "allowGray": false
        },
        {
          "type": "color",
          "messageKey": "MinuteColor",
          "defaultValue": "0x000000",
          "label": "Minute Color",
          "sunlight": false,
          "allowGray": false
        },
        {
          "type": "color",
          "messageKey": "MinuteOutlineColor",
          "defaultValue": "0xFFFFFF",
          "label": "Minute Outline Color",
          "sunlight": false,
          "allowGray": false
        },
        {
          "type": "toggle",
          "messageKey": "HourTickmarks",
          "label": "Display Hour Tickmarks",
          "defaultValue": true
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Other settings"
        },
        {
          "type": "toggle",
          "messageKey": "BatteryBar",
          "label": "Draw the battery state",
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "ShakeSpriteChange",
          "label": "Shake to Change the Sprite",
          "defaultValue": true
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
  ];