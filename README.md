# Background

https://www.welhat.gov.uk/rubbish-recycling/check-bin-collection-day is very annoying as it uses POST, so each time you want to check you need to enter your postcode and then select your address in the list.

It would be fine if this was a rare occurrence but the bin days are not fully fixed, they shift when there are bank holidays or bad weather or strike, etc so they need to be checked each week.

# Behaviour

This wakes up every day at 5pm and checks if bins will be picked up the next day (so you need to get them out tonight).

If not, it gets back to sleep.

If yes, it turns on the screen and shows which one to get out.

It gets back off in the evening, or if you press the screen to record you got them out.

# Example display

![Photo of the device showing the next bins](https://github.com/pterjan/esp32-welhat-bins/raw/main/example.jpg)

# You need a config.h with the following content

```
const char *ssid = "MyWiFi";
const char *password = "MyWiFiPassword";
const char *postcode = "Postcode in WGC";
const char *property = "Property ID";
```

You can get the property ID by inspecting the form after you have selected it on https://www.welhat.gov.uk/rubbish-recycling/check-bin-collection-day
