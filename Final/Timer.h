#ifndef CS5625_TIMER_H
#define CS5625_TIMER_H

class Timer {
public:
    Timer() : _lastTime(0), _playing(true), _rate(1), _lastRealtime(0) {
        time();
    }

    double time() {
        double t = (_playing ? 1 : 0) * _rate * timedelta() + _lastTime;
        _lastTime = t;
        return t;
    }

    void pause() {
        time();
        _playing = false;
    }

    void play() {
        time();
        _playing = true;
    }

    bool playing() const {
        return _playing;
    }

    void setPlaying(bool playing) {
        time();
        _playing = playing;
    }

    double rate() const {
        return _rate;
    }

    void setRate(double rate) {
        time();
        _rate = rate;
    }

    void offset(double timedelta) {
        time();
        _lastTime += timedelta;
    }

private:
    double _lastTime;
    bool _playing;
    double _rate;

    double _lastRealtime;

    double timedelta() {
        double now = realtime();
        double td = now - _lastRealtime;
        _lastRealtime = now;
        return td;
    }

    static double realtime();
};


#endif //CS5625_TIMER_H
