## Theory of information and encoding

---
> ### Implemented in `C` and [`LogiSim`](http://www.cburch.com/logisim/)
> This is pretty much old simulator and it's support is discontinued as for now.
> There's a another program called Logisim-Evolution - it's a more up-to-date, but there are some issues opening old `.circ` files (in particular with displaying labels).
>
> Though i've found a fork which is somehow working and is crossplatform, even on latest versions of MacOS.
> Here's the link: https://github.com/laurensk/logisim-macos

> All things are implemented in C.
> Circuits are built in Logisim (as you could have guessed already).

---
# 1. [Hamming (7-bit) coder](./hamming)
- [ Hamming Encoder ](https://github.com/verityafter/theofi/blob/master/hamming/encoder.c)
- [ Hamming Decoder ](https://github.com/verityafter/theofi/blob/master/hamming/decoder.c)

# 2. [Iterative coder](./iterative)
- [ Iterative Encoder ](https://github.com/verityafter/theofi/blob/master/iterative/decoder.c)
- [ Iterative Decoder ](https://github.com/verityafter/theofi/blob/master/iterative/decoder.c)

# 3. [Cyclic coder](./cyclic)
- [ Cyclic Encoder ](https://github.com/verityafter/theofi/blob/master/cyclic/decoder.c)
- [ Cyclic Decoder ](https://github.com/verityafter/theofi/blob/master/cyclic/decoder.c)

---
