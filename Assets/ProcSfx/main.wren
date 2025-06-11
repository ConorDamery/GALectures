import "app" for App

class Osc {
    construct new(freq) {
        _freq = freq
        _phase = 0
    }

    freq { _freq }
    freq=(v) {_freq = v }

    phase { _phase }
    phase=(v) {_phase = v }

    sin(dt) {
        var out = _phase.sin
        _phase = (_phase + _freq * 2 * Num.pi * dt) % (2 * Num.pi)
        return out
    }
}

// Demo 1 - Dual Sine Wave Beat
class Demo1 {
    construct new() {
        _osc1 = Osc.new(440.0)
        _osc2 = Osc.new(442.0)
    }

    update(dt) {}

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 1 - Dual Sine Wave Beat")
            _osc1.freq = App.guiFloat("Freq1", _osc1.freq, 1, 1000)
            _osc2.freq = App.guiFloat("Freq2", _osc2.freq, 1, 1000)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        return (_osc1.sin(dt) + _osc2.sin(dt)) * 0.5
    }
}


// Demo 2 - Square Wave with PWM
class Demo2 {
    construct new() {
        _osc = Osc.new(220.0)
        _lfo = Osc.new(0.5)
    }

    update(dt) {}

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 2 - Square Wave with PWM (Pulse Width Modulation)")
            _osc.freq = App.guiFloat("Base Freq", _osc.freq, 20, 2000)
            _lfo.freq = App.guiFloat("PWM Rate", _lfo.freq, 0.1, 10)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        var duty = 0.3 + 0.2 * _lfo.sin(dt)  // LFO modulates pulse width
        var t = _osc.phase / (2 * Num.pi)   // normalized phase in [0, 1)
        var out = (t < duty) ? 1 : -1
        _osc.sin(dt)  // advance phase, discard output
        return out * 0.3
    }
}

// Demo 3 - AM Synth
class Demo3 {
    construct new() {
        _carrier = Osc.new(440.0)
        _mod = Osc.new(5.0)
    }

    update(dt) {}

    render() {
        if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
            App.guiText("Demo 3 - AM (Amplitude Modulation) Synth")
            _carrier.freq = App.guiFloat("Carrier Freq", _carrier.freq, 20, 2000)
            _mod.freq = App.guiFloat("Mod Freq", _mod.freq, 0.1, 20)
        }
        App.guiEndChild()
    }

    audio(sampleRate, dt) {
        var modSignal = _mod.sin(dt) * 0.5 + 0.5
        return _carrier.sin(dt) * modSignal * 0.4
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

    update(dt) {
    }

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

    update(dt) {}

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
        _time = _time + dt
        return _osc.sin(dt) * 0.3
    }
}

// Entry point
class Main {
    static init() {
        __vol = 1
        __curr = 0
        __demos = [
            Demo1.new(),
            Demo2.new(),
            Demo3.new(),
            Demo4.new(),
            Demo5.new()
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
