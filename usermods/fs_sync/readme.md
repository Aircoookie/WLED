## FS Sync usermod

Synchronizes image files (`.gif`, `.bmp`) on the WLED file system (LittleFS) between all instances that share a sync group. Upload an image to one instance and every instance in the same sync group picks it up automatically.

### How does it work?

- Each instance periodically scans its file system and builds a manifest (path, size, modification time and CRC32 of every file).
- The manifest checksum is announced to the subnet broadcast address on a dedicated UDP port (default 21325). Announcements carry the sender's sync groups and are only accepted by instances whose receive groups intersect — the same semantics as the built-in WLED state sync.
- An instance seeing a different manifest requests the full manifest, diffs it against its local file system and pulls every file that is missing locally or older than the remote copy. The newest modification time wins; ties are broken by the higher unit ID (last IP octet), so both sides always agree on the winner.
- Files are transferred in ~1300 byte UDP chunks, written to `<path>.part` and renamed over the target once size and CRC32 are verified.

### What is synced

Only image files — `.gif` and `.bmp`, the formats the "Image" effect can display (see `image_loader.cpp`). The extension match is case-insensitive and applies to files in subdirectories as well.

Everything else on the file system is never sent or received: the device configuration (`cfg.json`), security settings (`wsec.json`, which holds the WiFi passwords), presets, WLED backup files (`/bkp.*`) and temporary files (including this usermod's own `.part` files).

File **deletions** are not propagated: deleting a file on one instance does not delete it on the others.

### Configuration

On the Usermod Settings page:

| Setting | Default | Description |
|---|---|---|
| enabled | on | Master switch for the usermod. |
| port | 21325 | UDP port. Must not be 21324 or 65506 (used by WLED itself). |
| scan | 60 | File system scan interval in seconds. |
| pass | (empty) | Optional shared passphrase. When set, packets carry a CRC32 based tag and instances only sync when the passphrase matches. |

Status (peers, files, sent/received counters, errors) is shown on the Info page (`/json/info` under `u` → `FS Sync`). A manual rescan can be triggered via the JSON API: `{"FS Sync":{"syncNow":true}}`.

### Requirements

- All instances must run firmware with this usermod enabled, be on the same LAN subnet, and share a sync group (Sync settings → send/receive groups), exactly like the built-in state sync.
- NTP time is recommended: modification times drive conflict resolution. Without it, ties are broken deterministically by unit ID, so sync still converges.

### Limitations

- LittleFS has no change notifications; changes are detected by the periodic scan (or the `syncNow` API call).
- Concurrent edits of the same file on two instances converge to the newest modification time; a file being written while it is transferred may fail the CRC check and is retried on the next cycle.
- Flash wear: files are only written when their content actually differs.

### Security note

Like WLED's own UDP sync, this protocol is unauthenticated on the LAN (see `docs/hardening.instructions.md` — protocol-defined UDP broadcast channels are accepted in WLED's firewall-isolated LAN threat model). The optional passphrase adds a CRC32 based packet tag so unrelated instances (e.g. on a shared network) do not sync by accident; it is not strong authentication. Sync groups already scope traffic to instances you chose to sync with.
