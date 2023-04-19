#pragma once
#include "Tool.h"

// 查找数组中的最大值(目标数组, 数组长度);
int TOOL::findArrMax(int arr[], int n) {
    int max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

// 查找数组中的最小值(目标数组, 数组长度);
int TOOL::findArrMin(int arr[], int n) {
    int min = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < min) {
            min = arr[i];
        }
    }
    return min;
}

String TOOL::XOR_encrypt(String plaintext, String key) {
    String ciphertext = "";
    unsigned char j = 0;
    for (char c : plaintext) {
        ciphertext += char(c ^ key[j]);
        j = (j + 1) % key.length();
    }
    return ciphertext;
}