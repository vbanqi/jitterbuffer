#ifndef __HOLA_JITTER_BUFFER_H_INCLUDED__
#define __HOLA_JITTER_BUFFER_H_INCLUDED__


class RtpData;
class OutData{
    public:
        virtual void OnOutData(RtpData *dt) = 0;
};

class JitterBuffer
{
public:
	JitterBuffer(int cacheSize, uint64_t waitTimer, OutData *out);
	virtual ~JitterBuffer();

    ngx_int_t InsertIntoCache(uint16_t seq, RtpData *dt);
private:
    bool IsStartInRang(int start, int rang1, int rang2);
    int PostAndRemove(int start, int end, bool sendAll);
    void SetCacheTime();

private:
    struct BoxData{
        RtpData *data;
        uint64_t rtime;
        bool isSend;

        BoxData()
            : data(nullptr)
            , rtime(0)
            , isSend(true){}
    };

    BoxData **m_jitterQueue;
    OutData *m_outData;
    int m_kMaxWait;
    int m_kCacheSize;
    int m_end;
    int m_start;
    uint16_t m_oriseq;
    uint16_t m_testori;
    bool m_init;
}; // class JitterBuffer

#endif // ifndef __HOLA_JITTER_BUFFER_H_INCLUDED__

