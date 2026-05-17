# Software — React Native Mobile Application

This folder contains the ThermoGo mobile application built with **React Native** (Expo managed workflow), targeting both **iOS** and **Android**.

## Contents

```
Software/
├── src/
│   ├── screens/
│   │   ├── HomeScreen.js       # BLE connect + live temperature display
│   │   ├── HistoryScreen.js    # Timestamped readings log
│   │   ├── ChartScreen.js      # Temperature trend graph
│   │   └── SettingsScreen.js   # Thresholds, units, user profile
│   ├── components/
│   │   ├── TemperatureCard.js  # Live temp display card
│   │   ├── AlertBanner.js      # Fever / hypothermia alert
│   │   └── DeviceScanner.js    # BLE device list
│   ├── services/
│   │   └── BLEService.js       # BLE scan, connect, notify
│   └── navigation/
│       └── AppNavigator.js     # Bottom tab navigation
├── app.json                    # Expo configuration
├── package.json
└── README.md
```

## Prerequisites

- Node.js >= 18.x
- npm >= 9.x
- Expo CLI — `npm install -g expo-cli`
- Android Studio (for Android) or Xcode (for iOS)

## Setup & Run

```bash
# Install dependencies
npm install

# Start Expo dev server
npx expo start

# Run on Android emulator or device
npx expo run:android

# Run on iOS simulator or device
npx expo run:ios
```

## App Screens

| Screen | Description |
|---|---|
| Home | Scan for ThermoGo device, trigger measurement, view live temperature |
| History | Scrollable list of past readings with date and time |
| Chart | Line graph of temperature over time |
| Settings | Set fever threshold, toggle °C/°F, manage user profile |

## BLE Integration

The app scans for the ThermoGo custom GATT service UUID, connects automatically when in range, and subscribes to the temperature characteristic. Readings are stored locally with **AsyncStorage**.

## Key Dependencies

| Package | Purpose |
|---|---|
| `react-native-ble-plx` | BLE communication |
| `@react-navigation/native` | Screen navigation |
| `react-native-chart-kit` | Temperature trend charts |
| `@react-native-async-storage/async-storage` | Local data persistence |
| `expo-notifications` | Push alerts for fever events |
