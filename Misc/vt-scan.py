#
# This script uploads a file to VirusTotal for scanning and retrieves the scan results
# Note that it requires a VirusTotal API key, either as an argument or in the VT_API_KEY environment variable.
# The script will exit with the following codes:
# 1 - Incorrect usage
# 2 - Malicious or suspicious detections
# 3 - Timeouts or failures
#
# Usage: python vt-scan.py <FILE_PATH> [API_KEY]
# Example: python vt-scan.py pwsafe.exe YOUR_API_KEY
#
# Copyright (c) 2025 Rony Shapiro <ronys@pwsafe.org>
# Generated with the assistance of Microsoft Copilot
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php

import requests
import sys
import time
import os

def upload_and_scan_file(api_key, file_path):
    url = "https://www.virustotal.com/api/v3/files"
    headers = {
        "x-apikey": api_key
    }

    try:
        with open(file_path, "rb") as file:
            files = {"file": (file_path, file)}
            response = requests.post(url, headers=headers, files=files)
            response.raise_for_status()
    except Exception as e:
        print(f"Error: {e}")
        return None

    response_data = response.json()
    analysis_id = response_data.get("data", {}).get("id", "")
    if not analysis_id:
        print("Error: Could not retrieve analysis ID.")
        return None

    print(f"File uploaded successfully! Analysis ID: {analysis_id}")
    return analysis_id

def get_scan_results(api_key, analysis_id):
    url = f"https://www.virustotal.com/api/v3/analyses/{analysis_id}"
    headers = {
        "x-apikey": api_key
    }

    while True:
        response = requests.get(url, headers=headers)
        response_data = response.json()

        status = response_data.get("data", {}).get("attributes", {}).get("status", "")
        if status == "completed":
            return response_data
        elif status == "queued":
            print("Scan is still in progress. Retrying in 10 seconds...")
            time.sleep(10)
        else:
            print("Error: Scan failed or unexpected status.")
            return None

def pretty_print_stats(stats):
    print("\nScan Results Summary:")
    print("-" * 30)

    # Red for malicious or suspicious detections
    malicious = stats.get('malicious', -1)
    suspicious = stats.get('suspicious', -1)
    if malicious > 0 or suspicious > 0:
        print(f"\033[91mMalicious detections: {malicious}\033[0m")
        print(f"\033[91mSuspicious detections: {suspicious}\033[0m")
        sys.exit(2)

    # Yellow for timeouts or failures
    timeout = stats.get('timeout', -1)
    confirmed_timeout = stats.get('confirmed-timeout', -1)
    failure = stats.get('failure', -1)
    if timeout > 0 or confirmed_timeout > 0 or failure > 0:
        print(f"\033[93mTimeout occurrences: {timeout}\033[0m")
        print(f"\033[93mConfirmed timeouts: {confirmed_timeout}\033[0m")
        print(f"\033[93mFailures: {failure}\033[0m")
        sys.exit(3)

    # Default print for other stats
    print(f"Malicious detections: {malicious}")
    print(f"Suspicious detections: {suspicious}")
    print(f"Undetected detections: {stats.get('undetected', -1)}")
    print(f"Harmless detections: {stats.get('harmless', -1)}")
    print(f"Timeout occurrences: {timeout}")
    print(f"Confirmed timeouts: {confirmed_timeout}")
    print(f"Failures: {failure}")
    print(f"Unsupported file type reports: {stats.get('type-unsupported', -1)}")
    print("-" * 30)

def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: python script.py <FILE_PATH> [API_KEY]")
        sys.exit(1)

    file_path = sys.argv[1]
    api_key = sys.argv[2] if len(sys.argv) == 3 else os.getenv("VT_API_KEY")

    if not api_key:
        print("Error: API key not provided. Please set the VT_API_KEY environment variable or provide it as an argument.")
        sys.exit(1)

    print("Uploading file...")
    analysis_id = upload_and_scan_file(api_key, file_path)
    if not analysis_id:
        sys.exit(1)

    print("Getting scan results...")
    results = get_scan_results(api_key, analysis_id)
    if not results:
        sys.exit(1)

    stats = results.get("data", {}).get("attributes", {}).get("stats", {})
    pretty_print_stats(stats)

if __name__ == "__main__":
    main()
