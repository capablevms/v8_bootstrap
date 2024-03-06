# Chrompartments' Hybrid Adapted V8 

This repo contains our work adapting V8 with hybrid CHERI to enhance some of
its security properties around memory.

The chromium version we work with is `121.0.6167.160` — it needs to be pinned
so that the correct versions of dependencies can be installed.

## Getting Started

There are some notes at the top of `CMakeLists.txt`, which include instructions
you'll need to act on before building V8. You should follow those before
bootstrapping the V8 source! The steps to get set up and build a helloworld
binary are…:

```sh
# === follow prerequisite steps from CMakeLists.txt ===
./grab_source.sh    # Sets up chromium source in this repo
cmake .             # Create makefiles
make                # Build v8. This takes a while, and there's a manual step!
make hello-world    # Compile a helloworld binary at `out/hybrid/helloworld`
```

## Planned approaches to introducing CHERI to V8

There are a couple of angles we're planning to take to introduce CHERI to V8,
and make things more secure in the process:

- "CapabilityHandles", a version of DirectHandles which may replace the lookup
  tables used by V8's sandbox to reduce overhead while maintaining its existing
  security properties.
- Using hybrid CHERI compartmentalisation to sandbox the execution of generated
  code.
- Construction of a "sandbox stack" where untrusted code might execute.

These are discussed in more detail
[in a google doc](https://docs.google.com/document/d/1m5wNFtEoji22sHvP9aTdFR9V-1GU25iRF9xQBd5Qv8o/edit?usp=sharing),
and relate to the V8 Sandbox team's
[notes on the possiblility of a hardware-supported sandbox](https://docs.google.com/document/d/12MsaG6BYRB-jQWNkZiuM3bY8X2B2cAsCMLLdgErvK4c/edit#heading=h.xzptrog8pyxf).
