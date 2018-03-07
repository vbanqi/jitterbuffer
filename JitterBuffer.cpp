#include "Utils.h"

#include "JitterBuffer.h"

#include "RtpData.h"


namespace hola
{

JitterBuffer::JitterBuffer(int cacheSize, uint64_t waitTimer, OutData *out)
    : m_outData(out)
    , m_kMaxWait(waitTimer)
    , m_kCacheSize(cacheSize)
    , m_end(-1)
    , m_start(-1)
    , m_oriseq(0)
    , m_testori(0)
{
    m_jitterQueue = new BoxData*[m_kCacheSize];
    for (int i = 0; i < m_kCacheSize; i++) {
        m_jitterQueue[i] = new BoxData();
    }
    //ngx_memzero(m_jitterQueue, m_kCacheSize * sizeof(RtpData *));
}

JitterBuffer::~JitterBuffer()
{
    for (int i = 0; i < m_kCacheSize; i++) {
        delete m_jitterQueue[i]->data;
        delete m_jitterQueue[i];
    }
    delete [] m_jitterQueue;
}

ngx_int_t JitterBuffer::InsertIntoCache(uint16_t seq, RtpData *dt)
{
    int oriInd = seq % m_kCacheSize;
    BoxData *box = m_jitterQueue[oriInd];
    if (m_end == -1) {//init
        m_end = oriInd;
        m_start = m_end;
        m_oriseq = seq;
        assert(box->isSend == true);
        box->data = dt;
        box->isSend = false;
        box->rtime = Utils::SystemTimesMillis();
    }
    else {
        if (Utils::IsMoreThan(seq, m_oriseq)) {
            box->rtime = Utils::SystemTimesMillis();
            if (Utils::GetDistance(seq, m_oriseq) >= m_kCacheSize) {
                m_start = PostAndRemove(m_start, m_end, true);
                if (m_jitterQueue[m_start]->isSend == false) {
                    m_outData->OnOutData(m_jitterQueue[m_start]->data);
                    m_jitterQueue[m_start]->isSend = true;
                    m_jitterQueue[m_start]->data = nullptr;
                }
                assert(box->data == nullptr);
                box->data = dt;
                box->isSend = false;
                box->rtime = Utils::SystemTimesMillis();

                m_end = oriInd;
                m_oriseq = seq;
                m_start = (m_end + 1) % m_kCacheSize; // move start to end cache
                SetCacheTime();
            }
            else if (m_start == m_end) {
                if (box->isSend == false) {
                    return NGX_ERROR;
                }
                box->data = dt;
                box->isSend = false;
                box->rtime = Utils::SystemTimesMillis();
                m_end = oriInd;
                m_oriseq = seq;
                SetCacheTime();
                m_start = PostAndRemove(m_start, m_end, false);
            }
            else {
                if (IsStartInRang(m_start, m_end, oriInd)) {
                    m_start = PostAndRemove(m_start, oriInd, true);
                }
                if (m_start == oriInd) {
                    if (m_jitterQueue[m_start]->isSend == false) {
                        m_outData->OnOutData(m_jitterQueue[m_start]->data);
                        m_jitterQueue[m_start]->isSend = true;
                        m_jitterQueue[m_start]->data = nullptr;
                    }
                    m_start++;
                    m_start %= m_kCacheSize;
                }
                if (box->isSend == false) {
                    return NGX_ERROR;
                }
                box->data = dt;
                box->isSend = false;
                box->rtime = Utils::SystemTimesMillis();
                m_end = oriInd;
                m_oriseq = seq;
                SetCacheTime();
                m_start = PostAndRemove(m_start, m_end, false);
            }
        }
        else {
            if (Utils::GetDistance(seq, m_oriseq) >= m_kCacheSize) {
                //discard
                return NGX_ERROR;
            }
            else if (m_start == m_end) {
                if (box->isSend == false) {
                    return NGX_ERROR;
                }
                return NGX_ERROR;
            }
            else {
                if (box->isSend == false) {
                    //discard
                    return NGX_ERROR;
                }
                else if(IsStartInRang(oriInd, m_start, m_end)) {
                    box->data = dt;
                    box->isSend = false;
                    box->rtime = Utils::SystemTimesMillis();
                    m_start = PostAndRemove(m_start, m_end, false);
                }
                else {
                    assert(box->data == nullptr);
                    return NGX_ERROR;
                }
            }
        }
    }
    return NGX_OK;
}

bool JitterBuffer::IsStartInRang(int start, int rang1, int rang2)
{
    int cur = rang1;
    while(cur != rang2) {
        if (start == cur) {
            return true;
        }
        cur ++;
        cur %= m_kCacheSize;
    }
    return false;
}

int JitterBuffer::PostAndRemove(int start, int end, bool sendAll)
{
    int cur = start;
    if (cur == end) {
        if (m_jitterQueue[cur]->isSend == false) {
            m_outData->OnOutData(m_jitterQueue[cur]->data);
            m_jitterQueue[cur]->isSend = true;
            m_jitterQueue[cur]->data = nullptr;
        }
        cur ++;
        cur %= m_kCacheSize;
        return cur;
    }
    while(cur != end) {
        if (m_jitterQueue[cur]->isSend == false) { 
            m_outData->OnOutData(m_jitterQueue[cur]->data);
            m_jitterQueue[cur]->isSend = true;
            m_jitterQueue[cur]->data = nullptr;
        }
        else if (m_jitterQueue[cur]->rtime + m_kMaxWait < Utils::SystemTimesMillis()) {
        }
        else if(!sendAll) {
            return cur;
        }
        cur ++;
        cur %= m_kCacheSize;
    }
    return end;
    //post to next step
}
void JitterBuffer::SetCacheTime()
{
    int end = m_end - 1;
    if (end < 0) {
        end = m_kCacheSize - 1;
    }
    while(end != m_start) {
        if (m_jitterQueue[end]->isSend == true) {
            m_jitterQueue[end]->rtime = Utils::SystemTimesMillis();
        }
        else
            break;
        end --;
        if (end < 0) {
            end = m_kCacheSize - 1;
        }
    }
}
} // namespace hola

