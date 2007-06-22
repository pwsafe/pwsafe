// Created on 22.06.2007
package org.pwsafe.passwordsafeswt.util;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * @author shamilbi
 */
public class IOUtils
{

    private static final Log LOG = LogFactory.getLog(IOUtils.class);

    public static void closeQuietly(InputStream in)
    {
        if (in == null)
        {
            return;
        }

        try
        {
            in.close();
        } catch (IOException e)
        {
            LOG.warn("InputStream.close() error", e);
        }
    }

    public static void close(OutputStream out) throws IOException
    {
        if (out == null)
        {
            return;
        }
        out.close();
    }

    public static void closeQuietly(OutputStream out)
    {
        try
        {
            close(out);
        } catch (IOException e)
        {
            LOG.warn("OutputStream.close() error", e);
        }
    }

    private IOUtils()
    {
        // only static methods
    }

}
