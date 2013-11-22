#include "audioclass.h"
#include <iostream>

static char const dest_map_mf_ss[] = {
        0, 1, 2, 3, 4, 5, 6, 7, /* analog */
        16, 17, 18, 19, 20, 21, 22, 23, /* adat */
        24, 25, /* spdif */
        26, 27 /* phones */
};

static char const channel_map_mf_ss[26] = {
        0, 1, 2, 3, 4, 5, 6, 7,         /* Line in */
        16, 17, 18, 19, 20, 21, 22, 23, /* ADAT */
        24, 25,                         /* SPDIF */
        26, 27,             /* Phones L+R, only a destination channel */
        -1, -1, -1, -1, -1, -1
};

static char const *labels_mf_ss[20] = {
    "AN 1", "AN 2", "AN 3", "AN 4", "AN 5", "AN 6", "AN 7", "AN 8",
    "A 1", "A 2", "A 3", "A 4", "A 5", "A 6", "A 7", "A 8",
    "SP.L", "SP.R", "AN.L", "AN.R"
};


std::string AudioClass::getDestName(int dest)
{
    std::string ret = labels_mf_ss[dest];
    return ret;
}

std::string AudioClass::getSourceName(int source)
{
    std::string ret;

    if (source < getSourceChannels()/2) {
        ret = labels_mf_ss[source];
    } else {
        ret = "P " + (source-getSourceChannels()/2);
        std::cout << ret << std::endl;
    }

    return ret;
}

void AudioClass::close()
{
    if (m_handle)
        snd_ctl_close(m_handle);
}

void AudioClass::complain(int err)
{
        //std::cerr << "ALSA error:" << snd_strerror(err) << std::endl;
}

void AudioClass::open(char *cardname)
{
    int err;


    if ((err = snd_ctl_open(&m_handle, "hw:DSP", 0)) < 0) {
        complain(err);
        exit (1);
    }
}

void AudioClass::open()
{
    open((char*)"hw:DSP");
}

int AudioClass::getPlaybackOffset() { return 26; };

int AudioClass::sourceToALSA(int source)
{
    if (source < getSourceChannels()/2) {
        /* input channels */
        return channel_map_mf_ss[source];
    } else {
        /* playback channels */
        return getPlaybackOffset()+channel_map_mf_ss[source-getSourceChannels()/2];
    }
};

int AudioClass::getSourceChannels()
{
    return 18*2;
}

int AudioClass::getDestChannels()
{
    return 10*2;
}

long int AudioClass::getGain(int source, int dest)
{
    int err;
    long int ret;

    snd_ctl_elem_id_t *m_id;
    snd_ctl_elem_value_t *m_ctl;

    snd_ctl_elem_value_alloca(&m_ctl);
    snd_ctl_elem_id_alloca(&m_id);
    snd_ctl_elem_id_set_name(m_id, "Mixer");
    snd_ctl_elem_id_set_interface(m_id, SND_CTL_ELEM_IFACE_HWDEP);
    snd_ctl_elem_id_set_device(m_id, 0);
    snd_ctl_elem_id_set_index(m_id, 0);
    snd_ctl_elem_value_set_id(m_ctl, m_id);

    snd_ctl_elem_value_set_integer(m_ctl, 0, sourceToALSA(source));
    snd_ctl_elem_value_set_integer(m_ctl, 1, dest_map_mf_ss[dest]);

    if ((err = snd_ctl_elem_read(m_handle, m_ctl)) < 0) {
        complain(err);
        ret = -1;
    } else {
        ret = snd_ctl_elem_value_get_integer(m_ctl, 2);
    }

    //snd_ctl_elem_id_free(m_id);
    //snd_ctl_elem_value_free(m_ctl);

    return ret;

}

double AudioClass::getGaindB(int source, int dest)
{
    long int gain = getGain(source, dest);
    //double db = 20.0 * log10(gain);
    return 0.0;
}



AudioClass::AudioClass()
{
}

AudioClass::~AudioClass()
{
    close();
}