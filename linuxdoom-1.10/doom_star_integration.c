/**
 * Doom - OASIS STAR API Integration Implementation
 */

#include "doom_star_integration.h"
#include "doomdef.h"
#include "info.h"  // For SPR_* constants
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static star_api_config_t g_star_config;
static boolean g_star_initialized = false;

// Map Doom sprite to keycard type (using enum values from info.h)
static int GetKeycardTypeFromSprite(int sprite) {
    // These match the enum values from info.h
    if (sprite == SPR_RKEY) return 1;  // Red keycard
    if (sprite == SPR_BKEY) return 2;  // Blue keycard
    if (sprite == SPR_YKEY) return 3;  // Yellow keycard
    if (sprite == SPR_BSKU || sprite == SPR_YSKU || sprite == SPR_RSKU) return 4;  // Skull key
    return 0;
}

// Map keycard type to item name
static const char* GetKeycardName(int keycard_type) {
    switch (keycard_type) {
        case 1: return DOOM_ITEM_RED_KEYCARD;
        case 2: return DOOM_ITEM_BLUE_KEYCARD;
        case 3: return DOOM_ITEM_YELLOW_KEYCARD;
        case 4: return DOOM_ITEM_SKULL_KEY;
        default: return NULL;
    }
}

// Map keycard type to description
static const char* GetKeycardDescription(int keycard_type) {
    switch (keycard_type) {
        case 1: return "Red Keycard - Opens red doors";
        case 2: return "Blue Keycard - Opens blue doors";
        case 3: return "Yellow Keycard - Opens yellow doors";
        case 4: return "Skull Key - Opens skull-marked doors";
        default: return "Unknown Keycard";
    }
}

void Doom_STAR_Init(void) {
    g_star_config.base_url = "https://star-api.oasisplatform.world/api";
    g_star_config.api_key = getenv("STAR_API_KEY");
    g_star_config.avatar_id = getenv("STAR_AVATAR_ID");
    g_star_config.timeout_seconds = 10;
    
    star_api_result_t result = star_api_init(&g_star_config);
    if (result != STAR_API_SUCCESS) {
        printf("STAR API: Failed to initialize: %s\n", star_api_get_last_error());
        return;
    }
    
    // Try SSO authentication first
    const char* username = getenv("STAR_USERNAME");
    const char* password = getenv("STAR_PASSWORD");
    
    if (username && password) {
        result = star_api_authenticate(username, password);
        if (result == STAR_API_SUCCESS) {
            g_star_initialized = true;
            printf("STAR API: Authenticated via SSO. Cross-game features enabled.\n");
            return;
        } else {
            printf("STAR API: SSO authentication failed: %s\n", star_api_get_last_error());
        }
    }
    
    // Fall back to API key
    if (g_star_config.api_key && g_star_config.avatar_id) {
        g_star_initialized = true;
        printf("STAR API: Initialized with API key. Cross-game features enabled.\n");
    } else {
        printf("STAR API: Warning - No authentication configured.\n");
    }
}

void Doom_STAR_Cleanup(void) {
    if (g_star_initialized) {
        star_api_cleanup();
        g_star_initialized = false;
        printf("STAR API: Cleaned up.\n");
    }
}

void Doom_STAR_OnKeycardPickup(int keycard_type) {
    if (!g_star_initialized) {
        return;
    }
    
    const char* keycard_name = GetKeycardName(keycard_type);
    const char* keycard_desc = GetKeycardDescription(keycard_type);
    
    if (!keycard_name) {
        return;
    }
    
    star_api_result_t result = star_api_add_item(
        keycard_name,
        keycard_desc,
        "Doom",
        "KeyItem"
    );
    
    if (result == STAR_API_SUCCESS) {
        printf("STAR API: Added %s to cross-game inventory.\n", keycard_name);
    } else {
        printf("STAR API: Failed to add %s: %s\n", keycard_name, star_api_get_last_error());
    }
}

boolean Doom_STAR_CheckDoorAccess(int door_line, int required_key) {
    if (!g_star_initialized) {
        return false;
    }
    
    const char* required_keycard = GetKeycardName(required_key);
    if (required_keycard) {
        if (star_api_has_item(required_keycard)) {
            printf("STAR API: Door opened using cross-game keycard: %s\n", required_keycard);
            star_api_use_item(required_keycard, "doom_door");
            return true;
        }
    }
    
    return false;
}

void Doom_STAR_OnItemPickup(const char* item_name, const char* item_description) {
    if (!g_star_initialized || !item_name) {
        return;
    }
    
    star_api_result_t result = star_api_add_item(
        item_name,
        item_description ? item_description : "Item from Doom",
        "Doom",
        "Miscellaneous"
    );
    
    if (result == STAR_API_SUCCESS) {
        printf("STAR API: Added %s to cross-game inventory.\n", item_name);
    }
}

boolean Doom_STAR_HasCrossGameKeycard(const char* keycard_name) {
    if (!g_star_initialized || !keycard_name) {
        return false;
    }
    
    return star_api_has_item(keycard_name) ? true : false;
}

