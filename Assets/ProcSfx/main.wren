import "app" for App
import "gnum" for NumExt, Complex

class Osc {
    construct new(freq) {
        _freq = freq
        _phase = 0
        _normPhase = 0
        _z = Complex.new(1, 0)
    }

    freq { _freq }
    freq=(v) { _freq = v }

    phase { _phase }
    normPhase { _normPhase }

    sin { _z.i }
    cos { _z.r }
    complex { _z }
    
    square { _phase < Num.pi ? 1 : -1 }
    saw { 2 * _normPhase - 1 }
    triangle { (_normPhase < 0.5) ? (4 * _normPhase - 1) : (3 - 4 * _normPhase) }

    update(dt) {
        var pi2 = 2 * Num.pi
        var angle = _freq * pi2 * dt
        _z = _z * Complex.exp(angle)
        _phase = (_phase + angle) % pi2
        _normPhase = _phase / pi2
    }
}

// Demo 1 - Dual Sine Wave Beat
class Demo1 {
    construct new() {
        _osc1 = Osc.new(440.0)
        _osc2 = Osc.new(442.0)
    }

    update(dt) {
    }

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 1 - Dual Sine Wave Beat")
            _osc1.freq = App.guiFloat("Freq1", _osc1.freq, 1, 1000)
            _osc2.freq = App.guiFloat("Freq2", _osc2.freq, 1, 1000)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        _osc1.update(dt)
        _osc2.update(dt)
        return (_osc1.sin + _osc2.sin) * 0.5
    }
}

// Demo 2 - Square Wave with PWM
class Demo2 {
    construct new() {
        _osc = Osc.new(220.0)
        _lfo = Osc.new(0.5)
    }

    update(dt) {
    }

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 2 - Square Wave with PWM (Pulse Width Modulation)")
            _osc.freq = App.guiFloat("Base Freq", _osc.freq, 20, 2000)
            _lfo.freq = App.guiFloat("PWM Rate", _lfo.freq, 0.1, 10)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        _lfo.update(dt)
        _osc.update(dt)

        var duty = 0.3 + 0.2 * _lfo.sin
        var out = (_osc.normPhase < duty) ? 1 : -1
        return out * 0.3
    }
}

// Demo 3 - AM Synth
class Demo3 {
    construct new() {
        _carrier = Osc.new(440.0)
        _mod = Osc.new(5.0)
    }

    update(dt) {
    }

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 3 - AM (Amplitude Modulation) Synth")
            _carrier.freq = App.guiFloat("Carrier Freq", _carrier.freq, 20, 2000)
            _mod.freq = App.guiFloat("Mod Freq", _mod.freq, 0.1, 20)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        _carrier.update(dt)
        _mod.update(dt)

        var modSignal = _mod.sin * 0.5 + 0.5
        return _carrier.sin * modSignal * 0.4
    }
}

// Demo 4 - Simple Kick Drum Synth
class Demo4 {
    construct new() {
        _time = 0
        _base = 40.0
        _snap = 150.0
        _pitchDecay = 8.0
        _ampDecay = 6.0
        _clickLevel = 0.4
    }

    update(dt) {}

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 4 - Simple Kick Drum Synth")
            _snap = App.guiFloat("Snap Freq", _snap, 50, 1000)
            _base = App.guiFloat("Base Freq", _base, 10, 200)
            _pitchDecay = App.guiFloat("Pitch Decay", _pitchDecay, 1, 20)
            _ampDecay = App.guiFloat("Amp Decay", _ampDecay, 1, 20)
            _clickLevel = App.guiFloat("Click", _clickLevel, 0, 1)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        if (_time > 1) _time = 0

        var pitch = _snap * ((1 - _time * _pitchDecay).max(0)).pow(3) + _base
        var amp = (1 - _time * _ampDecay).max(0)
        var sine = (2 * Num.pi * pitch * _time).sin
        var transient = (_time < 0.01) ? 1 : 0
        var out = (sine * amp + transient * _clickLevel)

        _time = _time + dt
        return out * 0.9
    }
}

// Demo 5 - Simple Melody
class Demo5 {
    construct new() {
        _osc = Osc.new(261.63)
        _time = 0
        _speed = 2.0
        _scale = [261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88]
    }

    update(dt) {
    }

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 5 - Simple Melody")
            _speed = App.guiFloat("Note Rate", _speed, 0.5, 10)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        var index = ((_time * _speed) % _scale.count).floor
        _osc.freq = _scale[index]
        _osc.update(dt)
        _time = _time + dt
        return _osc.sin * 0.3
    }
}

// Demo 6 - Dubstep Growl (Simplified)
class BPF {
    construct new(freq, bw) {
        _freq = freq
        _bw = bw
        _z1 = 0
        _z2 = 0
    }

    freq=(v) { _freq = v }
    bw=(v) { _bw = v }

    process(x, sampleRate) {
        var omega = 2 * Num.pi * _freq / sampleRate
        var alpha = omega.sin * NumExt.sinh(2.log / 2 * _bw * omega / omega.sin)

        var b0 = alpha
        var b1 = 0
        var b2 = -alpha
        var a0 = 1 + alpha
        var a1 = -2 * omega.cos
        var a2 = 1 - alpha

        var y = (b0 / a0) * x + (b1 / a0) * _z1 + (b2 / a0) * _z2 - (a1 / a0) * _z1 - (a2 / a0) * _z2
        _z2 = _z1
        _z1 = y
        return y
    }
}

class Demo6 {
    construct new() {
        _carrier = Osc.new(55.0)     // Base growl tone
        _mod = Osc.new(50.0)        // Adds grit through phase modulation
        _lfo = Osc.new(2.5)          // Sweeps vowel filter
        _sub = Osc.new(27.5)         // Adds low-end body
        _form1 = BPF.new(500, 1)
        _form2 = BPF.new(1200, 1)
    }

    update(dt) {}

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 6 - Dubstep Vowel Growl")
            _carrier.freq = App.guiFloat("Carrier Freq", _carrier.freq, 30, 150)
            _mod.freq = App.guiFloat("Mod Freq", _mod.freq, 30, 300)
            _lfo.freq = App.guiFloat("Vowel LFO", _lfo.freq, 0.05, 3.0)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        _carrier.update(dt)
        _mod.update(dt)
        _lfo.update(dt)
        _sub.update(dt)

        var modDepth = 6.0
        var modSignal = _mod.sin * modDepth
        var modPhase = (_carrier.phase + modSignal) % (2 * Num.pi)
        var raw = modPhase.sin

        var sub = _sub.square * 0.3

        var lfo = (_lfo.normPhase * 2 * Num.pi).sin * 0.5 + 0.5

        _form1.freq = 500 + lfo * 800
        _form2.freq = 1200 + lfo * 600

        var y = _form1.process(raw, sampleRate)
        y = _form2.process(y, sampleRate)

        var distorted = NumExt.tanh(y * 3.0)

        return (sub + distorted) * 0.5
    }
}

// Entry point
class Main {
    static init() {
        __vol = 0.5
        __curr = 0
        __demos = [
            Demo1.new(),
            Demo2.new(),
            Demo3.new(),
            Demo4.new(),
            Demo5.new(),
            Demo6.new()
        ]
        App.sfxBindCallback()
    }

    static update(dt) {
        __demos[__curr].update(dt)
    }

    static render() {
        if (App.guiBeginChild("Main", 500, 100)) {
            __vol = App.guiFloat("Vol", __vol, 0, 1)
            __curr = App.guiInt("Demo", __curr, 0, __demos.count - 1)
        }
        App.guiEndChild()
        
        __demos[__curr].render()
    }

    static audio(sampleRate, dt) {
        var out = __demos[__curr].audio(sampleRate, dt)
        return out * __vol
    }
}