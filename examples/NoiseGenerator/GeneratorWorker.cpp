//
//  GeneratorWorker.cpp
//  gtf
//
//  Created by David Gallardo on 29/05/16.
//  Copyright (c) 2016 GTF Development Group. All rights reserved.
//

#include "GeneratorWorker.h"
#include "PerlinNoise.h"
#include "GTFGradient.h"

#include <cmath>

bool GeneratorWorker::update(GeneratorInfo& info)
{
    if(info.dirty)
    {
        if(m_running)
        {
            m_cancel = true;
            m_workingThread.join();
            delete [] m_info.image;
        }
        
        m_info = info;
        m_ready = false;
        m_running = true;
        m_cancel = false;
        info.dirty = false;
        m_info.image = new unsigned char[m_info.resX * m_info.resY * 3u];
        m_workingThread = std::thread(&GeneratorWorker::run, this);
    }
    
    if(m_running && m_ready)
    {
        m_workingThread.join();
        m_running = false;
        m_ready = false;
        info.image = m_info.image;
        return true;
    }
    
    return false;
}

void GeneratorWorker::run()
{
    PerlinNoise pn(m_info.seed);
    
    GTFGradient gradient;
    gradient.marks.clear();
    gradient.addMark(0.0f, GTFColor(0xA0793D));
    gradient.addMark(0.2f, GTFColor(0xAA8347));
    gradient.addMark(0.3f, GTFColor(0xB48D51));
    gradient.addMark(0.4f, GTFColor(0xBE975B));
    gradient.addMark(0.6f, GTFColor(0xC8A165));
    gradient.addMark(0.7f, GTFColor(0xD2AB6F));
    gradient.addMark(0.8f, GTFColor(0xDCB579));
    gradient.addMark(1.0f, GTFColor(0xE6BF83));
    
	unsigned int kk = 0;
	// Visit every pixel of the image and assign a color generated with Perlin noise
	for(unsigned int i = 0; i < m_info.resY; ++i) {     // y
		for(unsigned int j = 0; j < m_info.resX; ++j) {  // x
            
            if(m_cancel)
            {
                return;
            }
            
			double x = (double)j/((double)m_info.resX);
			double y = (double)i/((double)m_info.resY);
            
            double n = 0;
            
            if(!m_info.wood)
            {
                // Typical Perlin noise
                n = pn.noise(m_info.panX * m_info.density * x, m_info.panY *  m_info.density * y, m_info.panZ);
            }
            else
            {
                // Wood like structure
                n = m_info.density * pn.noise(m_info.panX  * x, m_info.panY * y, m_info.panZ);
                n = n - floor(n);
            }
			
            
			// Map the values to the [0, 255] interval, for simplicity we use
			// 50 shaders of grey
            GTFColor color = gradient.getColorAt(n);
            color.asU8A(&m_info.image[(kk*3)+0]);
            
            
			//m_info.image[(kk*3)+0] = floor(255 * n);
			//m_info.image[(kk*3)+1] = floor(255 * n);
			//m_info.image[(kk*3)+2] = floor(255 * n);
			kk++;
            
            
		}
	}
    
    m_ready = true;
}