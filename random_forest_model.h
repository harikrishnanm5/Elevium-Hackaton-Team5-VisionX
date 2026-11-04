#pragma once
#if defined(ARDUINO) || defined(ESP32) || defined(ESP8266)
#include <Arduino.h>
#else
#include <stdint.h>
#include <math.h>
#include <string.h>
#ifndef memcpy_P
#define memcpy_P memcpy
#endif
#endif
#include "model_trees.h"

// ============================================================================
// ELEVIUM - EDGE AI RANDOM FOREST INFERENCE ENGINE
// Team Vision X | Dual ML Architecture: 50-Tree Classifier + 150-Tree Regressor
// ============================================================================

#define NUM_FEATURES 8
#define CLASSIFIER_NUM_TREES 50
#define REGRESSOR_NUM_TREES 150

/**
 * @brief Traverses a single decision tree in PROGMEM given scaled features.
 * @param tree Pointer to the Node array in PROGMEM.
 * @param scaled_features Normalized input feature vector (8 features).
 * @return float Leaf node value (class ID for classifier, continuous value for regressor).
 */
static inline float evaluateTree(const Node* tree, const float scaled_features[NUM_FEATURES]) {
    int16_t curr = 0;
    while (curr >= 0) {
        Node node;
        #if defined(ESP32) || defined(ESP8266)
            // ESP32 supports direct memory mapped flash reads, but memcpy_P ensures portability
            memcpy_P(&node, &tree[curr], sizeof(Node));
        #else
            memcpy_P(&node, &tree[curr], sizeof(Node));
        #endif

        // Leaf condition: both child indices are -1
        if (node.left_child == -1 && node.right_child == -1) {
            return node.threshold_or_value;
        }

        // Branching condition
        int16_t f_idx = node.feature_idx;
        if (f_idx >= 0 && f_idx < NUM_FEATURES) {
            if (scaled_features[f_idx] <= node.threshold_or_value) {
                curr = node.left_child;
            } else {
                curr = node.right_child;
            }
        } else {
            // Guard against corrupted index
            break;
        }
    }
    return 0.0f;
}

/**
 * @brief Standardizes the raw 8-feature vector using scaler mean and scale parameters.
 * @param raw Raw feature vector [T1, T2, T3, T1-T2, T2-T3, T3-T1, Current, Voltage]
 * @param scaled Destination array for standardized features
 */
static inline void scaleFeatures(const float raw[NUM_FEATURES], float scaled[NUM_FEATURES]) {
    for (int i = 0; i < NUM_FEATURES; i++) {
        float mean = pgm_read_float(&scaler_mean[i]);
        float scale = pgm_read_float(&scaler_scale[i]);
        if (scale != 0.0f) {
            scaled[i] = (raw[i] - mean) / scale;
        } else {
            scaled[i] = 0.0f;
        }
    }
}

/**
 * @brief Predicts the Thermal Runaway Risk Class (0: NORMAL, 1: WARNING, 2: HIGH RISK).
 * Runs majority voting over the 50-tree Random Forest Classifier in PROGMEM.
 * 
 * @param features Raw 8-element feature array.
 * @return int Predicted risk level: 0 (Normal), 1 (Warning), 2 (High Risk), or -1 if invalid.
 */
inline int predictRiskClass(const float features[NUM_FEATURES]) {
    // Validate inputs
    for (int i = 0; i < NUM_FEATURES; i++) {
        if (isnan(features[i]) || isinf(features[i])) {
            return -1;
        }
    }

    float scaled[NUM_FEATURES];
    scaleFeatures(features, scaled);

    int votes[3] = {0, 0, 0};

    for (int t = 0; t < CLASSIFIER_NUM_TREES; t++) {
        const Node* tree_ptr = (const Node*)pgm_read_ptr(&clf_forest[t]);
        float pred = evaluateTree(tree_ptr, scaled);
        int class_id = (int)roundf(pred);
        if (class_id >= 0 && class_id <= 2) {
            votes[class_id]++;
        }
    }

    // Determine majority voted class
    int best_class = 0;
    int max_votes = votes[0];
    for (int c = 1; c <= 2; c++) {
        if (votes[c] > max_votes) {
            max_votes = votes[c];
            best_class = c;
        }
    }

    return best_class;
}

/**
 * @brief Estimates internal core temperature (°C) non-invasively using 150-tree RF Regressor.
 * 
 * @param features Raw 8-element feature array.
 * @return float Estimated internal core temperature in degrees Celsius.
 */
inline float predictCoreTemperature(const float features[NUM_FEATURES]) {
    // Validate inputs
    for (int i = 0; i < NUM_FEATURES; i++) {
        if (isnan(features[i]) || isinf(features[i])) {
            return NAN;
        }
    }

    float scaled[NUM_FEATURES];
    scaleFeatures(features, scaled);

    float sum_estimates = 0.0f;
    for (int t = 0; t < REGRESSOR_NUM_TREES; t++) {
        const Node* tree_ptr = (const Node*)pgm_read_ptr(&reg_forest[t]);
        sum_estimates += evaluateTree(tree_ptr, scaled);
    }

    return sum_estimates / (float)REGRESSOR_NUM_TREES;
}
