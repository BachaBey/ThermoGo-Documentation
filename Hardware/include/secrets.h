#pragma once

#define SUPABASE_URL    "https://cxanxrjmxvtckfkppmdt.supabase.co"
#define SUPABASE_KEY    "sb_publishable_O3EA2TF1Y-a-I2ET_HvEqw_YGM14us-"

// Default WiFi networks — always seeded into flash on first boot.
// Add up to 3 (leaves 2 slots free for app-added networks).
// Set SSID to "" to leave that slot unused.
struct DefaultNetwork { const char* ssid; const char* password; };
static const DefaultNetwork DEFAULT_NETWORKS[] = {
    { "ooredoo_9B246A_EXT", "qyiixorv"      },
    { "EVENT_SMU",          "$mUeV&nt2@25"  },
    { "TOPNET_55C0",        "nygcims04v"    },
};
