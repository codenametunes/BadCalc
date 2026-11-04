# BadCalc

A calculator that's just bad. On purpose.

>> Early development! No calculations YET

## What is this?

BadCalc is a deliberately terrible calculator. The goal is a calculator that looks normal and behaves *almost* correctly, until it doesn't:

- Floating-point chaos
- Off-by-one results
- Answers that drift every time you press equals

## What works right now

- Window and UI come up
- Buttons render and respond to clicks
- Display shows what you type

## What doesn't work (yet)

- Any actual calculation

## Tech

- Language: C
- UI: Win32 GDI
- IDE: Visual Studio 2022
- Platform Toolset - v141_xp (XP Support)
## Roadmap

- [x] Base window and UI
- [x] Button input and display
- [ ] Basic arithmetic (correct, at first)
- [ ] Introduce the bugs
- [ ] Drift on repeated `=`
- [ ] Floating-point chaos mode

## License

TBD

---

*If it gives you the right answer, that's the bug.*
