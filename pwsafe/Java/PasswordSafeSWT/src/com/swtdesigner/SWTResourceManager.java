package com.swtdesigner;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;

import org.eclipse.jface.resource.CompositeImageDescriptor;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.resource.ImageRegistry;
import org.eclipse.jface.resource.JFaceResources;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

/**
 * Utility class for managing OS resources associated with SWT controls such as
 * colors, fonts, images, etc.
 * 
 * !!! IMPORTANT !!! Application code must explicitly invoke the <code>dispose()</code>
 * method to release the operating system resources managed by cached objects
 * when those objects and OS resources are no longer needed (e.g. on
 * application shutdown)
 * 
 * This class may be freely distributed as part of any application or plugin.
 * <p>
 * Copyright (c) 2003 - 2005, Instantiations, Inc. <br>All Rights Reserved
 * 
 * @version $Revision$
 * @author scheglov_ke
 * @author Dan Rubel
 */
public class SWTResourceManager {

    /**
     * Dispose of cached objects and their underlying OS resources. This should
     * only be called when the cached objects are no longer needed (e.g. on
     * application shutdown)
     */
    public static void dispose() {
        disposeColors();
        disposeFonts();
        disposeImages();
        disposeCursors();
    }

    //////////////////////////////
    // Color support
    //////////////////////////////

    /**
     * Maps RGB values to colors
     */
    private static HashMap<RGB, Color> m_ColorMap = new HashMap<RGB, Color>();

    /**
     * Returns the system color matching the specific ID
     * @param systemColorID int The ID value for the color
     * @return Color The system color matching the specific ID
     */
    public static Color getColor(int systemColorID) {
        Display display = Display.getCurrent();
        return display.getSystemColor(systemColorID);
    }

    /**
     * Returns a color given its red, green and blue component values
     * @param r int The red component of the color
     * @param g int The green component of the color
     * @param b int The blue component of the color
     * @return Color The color matching the given red, green and blue componet values
     */
    public static Color getColor(int r, int g, int b) {
        return getColor(new RGB(r, g, b));
    }

    /**
     * Returns a color given its RGB value
     * @param rgb RGB The RGB value of the color
     * @return Color The color matching the RGB value
     */
    public static Color getColor(RGB rgb) {
        Color color = m_ColorMap.get(rgb);
        if (color == null) {
            Display display = Display.getCurrent();
            color = new Color(display, rgb);
            m_ColorMap.put(rgb, color);
        }
        return color;
    }

    /**
     * Dispose of all the cached colors
     */
    public static void disposeColors() {
        for (Iterator<Color> iter = m_ColorMap.values().iterator(); iter.hasNext();)
             iter.next().dispose();
        m_ColorMap.clear();
    }

    //////////////////////////////
    // Image support
    //////////////////////////////

	/**
	 * Maps image names to images
	 */
    private static HashMap<String, Image> m_ClassImageMap = new HashMap<String, Image>();

	/**
	 * Maps images to image decorators
	 */
    private static HashMap<Image, HashMap<Image, Image>> m_ImageToDecoratorMap = new HashMap<Image, HashMap<Image, Image>>();

    /**
     * Returns an image encoded by the specified input stream
     * @param is InputStream The input stream encoding the image data
     * @return Image The image encoded by the specified input stream
     */
    protected static Image getImage(InputStream is) {
        Display display = Display.getCurrent();
        ImageData data = new ImageData(is);
        if (data.transparentPixel > 0)
            return new Image(display, data, data.getTransparencyMask());
        return new Image(display, data);
    }

    /**
     * Returns an image stored in the file at the specified path
     * @param path String The path to the image file
     * @return Image The image stored in the file at the specified path
     */
    public static Image getImage(String path) {
    	return getImage("default", path); //$NON-NLS-1$
    }

    /**
     * Returns an image stored in the file at the specified path
     * @param section The section to which belongs specified image
     * @param path String The path to the image file
     * @return Image The image stored in the file at the specified path
     */
    public static Image getImage(String section, String path) {
        String key = section + "|" + SWTResourceManager.class.getName() + "|" + path;
//        Image image = m_ClassImageMap.get(key);
        final ImageRegistry imageRegistry = JFaceResources.getImageRegistry();
        Image image = imageRegistry.get(key);
        if (image == null) {
            try {
//                FileInputStream fis = new FileInputStream(path);
//                image = getImage(fis);
//                m_ClassImageMap.put(key, image);
//                fis.close();
                imageRegistry.put(key, ImageDescriptor.createFromURL(new URL(
                        "file:" + path)));
            } catch (Exception e) {
//              image = getMissingImage();
//            	m_ClassImageMap.put(key, image);
                return null;
            }
            image = imageRegistry.get(key);
        }
        return image;
    }

    /**
     * Returns an image stored in the file at the specified path relative to the specified class
     * @param clazz Class The class relative to which to find the image
     * @param path String The path to the image file
     * @return Image The image stored in the file at the specified path
     */
    public static Image getImage(Class<?> clazz, String path) {
        String key = clazz.getName() + '|' + path;
//        Image image = m_ClassImageMap.get(key);
        final ImageRegistry imageRegistry = JFaceResources.getImageRegistry();
        Image image = imageRegistry.get(key);        
        if (image == null) {
//        	try {
//        		if (path.length() > 0 && path.charAt(0) == '/') {
//        			String newPath = path.substring(1, path.length());
//        			image = getImage(new BufferedInputStream(clazz.getClassLoader().getResourceAsStream(newPath)));
//        		} else {
//        			image = getImage(clazz.getResourceAsStream(path));
//        		}
//        		m_ClassImageMap.put(key, image);
//        	} catch (Exception e) {
//        		image = getMissingImage();
//        		m_ClassImageMap.put(key, image);
//        	}
        	imageRegistry.put(key, ImageDescriptor.createFromFile(clazz, path));
            image = imageRegistry.get(key);
        }
        return image;
    }

    private static final int MISSING_IMAGE_SIZE = 10;
	private static Image getMissingImage() {
		Image image = new Image(Display.getCurrent(), MISSING_IMAGE_SIZE, MISSING_IMAGE_SIZE);
		//
		GC gc = new GC(image);
		gc.setBackground(getColor(SWT.COLOR_RED));
		gc.fillRectangle(0, 0, MISSING_IMAGE_SIZE, MISSING_IMAGE_SIZE);
		gc.dispose();
		//
		return image;
	}

    /**
     * Style constant for placing decorator image in top left corner of base image.
     */
    public static final int TOP_LEFT = 1;
    /**
     * Style constant for placing decorator image in top right corner of base image.
     */
    public static final int TOP_RIGHT = 2;
    /**
     * Style constant for placing decorator image in bottom left corner of base image.
     */
    public static final int BOTTOM_LEFT = 3;
    /**
     * Style constant for placing decorator image in bottom right corner of base image.
     */
    public static final int BOTTOM_RIGHT = 4;
    
    /**
     * Returns an image composed of a base image decorated by another image
     * @param baseImage Image The base image that should be decorated
     * @param decorator Image The image to decorate the base image
     * @return Image The resulting decorated image
     */
    public static Image decorateImage(Image baseImage, Image decorator) {
    	return decorateImage(baseImage, decorator, BOTTOM_RIGHT);
    }
    
    /**
	 * Returns an image composed of a base image decorated by another image
	 * @param baseImage Image The base image that should be decorated
	 * @param decorator Image The image to decorate the base image
	 * @param corner The corner to place decorator image
	 * @return Image The resulting decorated image
	 */
	public static Image decorateImage(final Image baseImage, final Image decorator, final int corner) {
		HashMap<Image, Image> decoratedMap = m_ImageToDecoratorMap.get(baseImage);
		if (decoratedMap == null) {
			decoratedMap = new HashMap<Image, Image>();
			m_ImageToDecoratorMap.put(baseImage, decoratedMap);
		}
		Image result = decoratedMap.get(decorator);
		if (result == null) {
			final Rectangle bid = baseImage.getBounds();
			final Rectangle did = decorator.getBounds();
            final Point baseImageSize = new Point(bid.width, bid.height); 
            CompositeImageDescriptor compositImageDesc = new CompositeImageDescriptor() { 
                protected void drawCompositeImage(int width, int height) { 
                    drawImage(baseImage.getImageData(), 0, 0); 
                    if (corner == TOP_LEFT) { 
                        drawImage(decorator.getImageData(), 0, 0); 
                    } else if (corner == TOP_RIGHT) { 
                        drawImage(decorator.getImageData(), bid.width - did.width - 1, 0); 
                    } else if (corner == BOTTOM_LEFT) { 
                        drawImage(decorator.getImageData(), 0, bid.height - did.height - 1); 
                    } else if (corner == BOTTOM_RIGHT) { 
                        drawImage(decorator.getImageData(), bid.width - did.width - 1, bid.height - did.height - 1); 
                    } 
                } 
                protected Point getSize() { 
                    return baseImageSize; 
                } 
            }; 
            result = compositImageDesc.createImage(); 
			decoratedMap.put(decorator, result);
		}
		return result;
	}

    /**
     * Dispose all of the cached images
     */
    public static void disposeImages() {
        for (Iterator<Image> I = m_ClassImageMap.values().iterator(); I.hasNext();)
             I.next().dispose();
        m_ClassImageMap.clear();
        //
        for (Iterator<HashMap<Image, Image>> I = m_ImageToDecoratorMap.values().iterator(); I.hasNext();) {
			HashMap<Image, Image> decoratedMap = I.next();
			for (Iterator<Image> J = decoratedMap.values().iterator(); J.hasNext();) {
				Image image = (Image) J.next();
				image.dispose();
			}
		}
    }

    /**
	 * Dispose cached images in specified section
	 * @param section the section do dispose
	 */
	public static void disposeImages(String section) {
		for (Iterator<String> I = m_ClassImageMap.keySet().iterator(); I.hasNext();) {
			String key = I.next();
			if (!key.startsWith(section + '|'))
				continue;
			Image image = m_ClassImageMap.get(key);
			image.dispose();
			I.remove();
		}
	}

    //////////////////////////////
    // Font support
    //////////////////////////////

    /**
     * Maps font names to fonts
     */
    private static HashMap<String, Font> m_FontMap = new HashMap<String, Font>();

    /**
     * Maps fonts to their bold versions
     */
    private static HashMap<Font, Font> m_FontToBoldFontMap = new HashMap<Font, Font>();

    /**
     * Returns a font based on its name, height and style
     * @param name String The name of the font
     * @param height int The height of the font
     * @param style int The style of the font
     * @return Font The font matching the name, height and style
     */
    public static Font getFont(String name, int height, int style) {
    	return getFont(name, height, style, false, false);
    }


    /**
     * Returns a font based on its name, height and style. 
     * Windows-specific strikeout and underline flags are also supported.
     * @param name String The name of the font
     * @param size int The size of the font
     * @param style int The style of the font
     * @param strikeout boolean The strikeout flag (warning: Windows only)
     * @param underline boolean The underline flag (warning: Windows only)
     * @return Font The font matching the name, height, style, strikeout and underline
     */
	public static Font getFont(String name, int size, int style, boolean strikeout, boolean underline) {
		String fontName = name + '|' + size + '|' + style + '|' + strikeout + '|' + underline;
        Font font = m_FontMap.get(fontName);
        if (font == null) {
        	FontData fontData = new FontData(name, size, style);
    		if (strikeout || underline) {
    			try {
    				Class<?> logFontClass = Class.forName("org.eclipse.swt.internal.win32.LOGFONT"); //$NON-NLS-1$
    				Object logFont = FontData.class.getField("data").get(fontData); //$NON-NLS-1$
    				if (logFont != null && logFontClass != null) {
    					if (strikeout) {
							logFontClass.getField("lfStrikeOut").set(logFont, new Byte((byte) 1)); //$NON-NLS-1$
						}
    					if (underline) {
							logFontClass.getField("lfUnderline").set(logFont, new Byte((byte) 1)); //$NON-NLS-1$
						}
    				}
    			} catch (Throwable e) {
    				System.err.println(
    					"Unable to set underline or strikeout" + " (probably on a non-Windows platform). " + e); //$NON-NLS-1$ //$NON-NLS-2$
    			}
    		}
    		font = new Font(Display.getCurrent(), fontData);
    		m_FontMap.put(fontName, font);
        }
		return font;
	}
    

    /**
     * Return a bold version of the give font
     * @param baseFont Font The font for whoch a bold version is desired
     * @return Font The bold version of the give font
     */
    public static Font getBoldFont(Font baseFont) {
        Font font = m_FontToBoldFontMap.get(baseFont);
        if (font == null) {
            FontData fontDatas[] = baseFont.getFontData();
            FontData data = fontDatas[0];
            font = new Font(Display.getCurrent(), data.getName(), data.getHeight(), SWT.BOLD);
            m_FontToBoldFontMap.put(baseFont, font);
        }
        return font;
    }

    /**
     * Dispose all of the cached fonts
     */
    public static void disposeFonts() {
        for (Iterator<Font> iter = m_FontMap.values().iterator(); iter.hasNext();)
             iter.next().dispose();
        m_FontMap.clear();
    }

	//////////////////////////////
    // CoolBar support
    //////////////////////////////

    /**
     * Fix the layout of the specified CoolBar
     * @param bar CoolBar The CoolBar that shgoud be fixed
     */
    public static void fixCoolBarSize(CoolBar bar) {
        CoolItem[] items = bar.getItems();
        // ensure that each item has control (at least empty one)
        for (int i = 0; i < items.length; i++) {
            CoolItem item = items[i];
            if (item.getControl() == null)
                item.setControl(new Canvas(bar, SWT.NONE) {
                @Override
				public Point computeSize(int wHint, int hHint, boolean changed) {
                    return new Point(20, 20);
                }
            });
        }
        // compute size for each item
        for (int i = 0; i < items.length; i++) {
            CoolItem item = items[i];
            Control control = item.getControl();
            control.pack();
            Point size = control.getSize();
            item.setSize(item.computeSize(size.x, size.y));
        }
    }

    //////////////////////////////
    // Cursor support
    //////////////////////////////

    /**
     * Maps IDs to cursors
     */
    private static HashMap<Integer, Cursor> m_IdToCursorMap = new HashMap<Integer, Cursor>();
 
    /**
     * Returns the system cursor matching the specific ID
     * @param id int The ID value for the cursor
     * @return Cursor The system cursor matching the specific ID
     */
    public static Cursor getCursor(int id) {
        Integer key = new Integer(id);
        Cursor cursor = m_IdToCursorMap.get(key);
        if (cursor == null) {
            cursor = new Cursor(Display.getDefault(), id);
            m_IdToCursorMap.put(key, cursor);
        }
        return cursor;
    }
 
    /**
     * Dispose all of the cached cursors
     */
    public static void disposeCursors() {
        for (Iterator<Cursor> iter = m_IdToCursorMap.values().iterator(); iter.hasNext();)
             iter.next().dispose();
        m_IdToCursorMap.clear();
    }
}