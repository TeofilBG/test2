#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// =============================================================================
// HARDWARE PIN CONFIGURATION
// ST77916 Display Controller with Audio and SD Card Support
// =============================================================================

// -----------------------------------------------------------------------------
// SD CARD PINS (MMC Mode)
// -----------------------------------------------------------------------------
#define SD_MMC_D0_PIN 2
#define SD_MMC_D1_PIN 6
#define SD_MMC_D2_PIN 6     // Note: D1 and D2 share the same pin
#define SD_MMC_D3_PIN 5
#define SD_MMC_CLK_PIN 3
#define SD_MMC_CMD_PIN 4

// SD Card Configuration
#define SD_CARD_MOUNT_POINT "/sd"
#define SD_CARD_MAX_FILES 10

// -----------------------------------------------------------------------------
// AUDIO OUTPUT PINS (I2S)
// -----------------------------------------------------------------------------
#define AUDIO_I2S_MCK_IO -1     // MCK (Master Clock) - Not used (-1)
#define AUDIO_I2S_BCK_IO 18     // BCK (Bit Clock)
#define AUDIO_I2S_WS_IO 16      // WS/LCK (Word Select/Left-Right Clock)
#define AUDIO_I2S_DO_IO 17      // DO/DIN (Data Out)
#define AUDIO_MUTE_PIN 48       // Mute Control Pin (LOW = mute, HIGH = unmute)

// Audio Configuration
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_CHANNELS 2        // Stereo
#define AUDIO_DMA_BUF_COUNT 8
#define AUDIO_DMA_BUF_LEN 1024

// -----------------------------------------------------------------------------
// MICROPHONE PINS (I2S Input)
// -----------------------------------------------------------------------------
#define MIC_I2S_WS 45          // Word Select
#define MIC_I2S_SD 46          // Serial Data
#define MIC_I2S_SCK 42         // Serial Clock

// Microphone Configuration
#define MIC_SAMPLE_RATE 16000
#define MIC_BITS_PER_SAMPLE 16
#define MIC_CHANNELS 1         // Mono
#define MIC_DMA_BUF_COUNT 4
#define MIC_DMA_BUF_LEN 512

// -----------------------------------------------------------------------------
// DISPLAY PINS (if needed for reference)
// -----------------------------------------------------------------------------
// Note: Display pins are typically handled by the display_panel.h module
// Add here if you need to reference them in other modules

// -----------------------------------------------------------------------------
// TOUCH PINS (if needed for reference)
// -----------------------------------------------------------------------------
// Note: Touch pins are typically handled by the display_panel.h module
// Add here if you need to reference them in other modules

// -----------------------------------------------------------------------------
// OTHER HARDWARE PINS
// -----------------------------------------------------------------------------
// Add any additional hardware pins here (LEDs, buttons, sensors, etc.)
// #define STATUS_LED_PIN 21
// #define USER_BUTTON_PIN 0

// -----------------------------------------------------------------------------
// HARDWARE FEATURE ENABLES
// -----------------------------------------------------------------------------
#define ENABLE_SD_CARD 1        // Enable SD card functionality
#define ENABLE_AUDIO_OUTPUT 1   // Enable I2S audio output
#define ENABLE_MICROPHONE 0     // Enable I2S microphone input (disabled by default)
#define ENABLE_AUDIO_MUTE 1     // Enable hardware mute control

// -----------------------------------------------------------------------------
// AUDIO FILE SUPPORT
// -----------------------------------------------------------------------------
#define SUPPORT_WAV_FILES 1     // Enable WAV file playback
#define SUPPORT_MP3_FILES 0     // Enable MP3 file playback (requires additional library)
#define MAX_AUDIO_FILE_SIZE (10 * 1024 * 1024)  // 10MB max file size

// Audio file paths
#define AUDIO_FILES_DIR "/sd/audio"
#define NOTIFICATION_SOUND "/sd/audio/notification.wav"
#define STARTUP_SOUND "/sd/audio/startup.wav"
#define ERROR_SOUND "/sd/audio/error.wav"

// -----------------------------------------------------------------------------
// HARDWARE VALIDATION MACROS
// -----------------------------------------------------------------------------
// Compile-time checks for pin conflicts
#if (SD_MMC_D1_PIN == SD_MMC_D2_PIN)
    #warning "SD_MMC_D1_PIN and SD_MMC_D2_PIN share the same pin (6)"
#endif

// Audio pin validation
#if (AUDIO_I2S_BCK_IO == AUDIO_I2S_WS_IO) || (AUDIO_I2S_BCK_IO == AUDIO_I2S_DO_IO) || (AUDIO_I2S_WS_IO == AUDIO_I2S_DO_IO)
    #error "Audio I2S pins must be unique"
#endif

// Microphone pin validation
#if (MIC_I2S_WS == MIC_I2S_SD) || (MIC_I2S_WS == MIC_I2S_SCK) || (MIC_I2S_SD == MIC_I2S_SCK)
    #error "Microphone I2S pins must be unique"
#endif

#endif // HARDWARE_CONFIG_H