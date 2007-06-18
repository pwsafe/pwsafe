package org.pwsafe.passwordsafeswt.xml;

import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

/**
 * Handles the import and export of passwordsafe files to XML.
 * 
 * @author Glen Smith
 */
public class XMLDataParser {

	private static final Log log = LogFactory.getLog(XMLDataParser.class);

	private static final int PARENT_NODE = 2;

	public static final String GROUP_TAG = "group";

	public static final String TREE_ATTR = "tree";

	public static final String TITLE_TAG = "title";

	public static final String USERNAME_TAG = "username";

	public static final String PASSWORD_TAG = "password";

	public static final String NOTES_TAG = "notes";

	public static final String ENTRY_TAG = "pwentry";

	public static final String LIST_TAG = "pwlist";

	/**
	 * This parser is responsible for parsing Token Data XML into a useable
	 * bean.
	 * 
	 */
	public static class XMLDataParserHandler extends DefaultHandler {

		Stack tagStack = new Stack();

		PwsEntryDTO entryDTO = new PwsEntryDTO();

		List entryList = new ArrayList();
		
		StringBuffer tagContent = new StringBuffer();

		/**
		 * @see org.xml.sax.helpers.DefaultHandler#startElement(java.lang.String,
		 *      java.lang.String, java.lang.String, org.xml.sax.Attributes)
		 */
		public void startElement(String uri, String localName, String qName,
				Attributes attrs) throws SAXException {
			tagStack.push(qName);
			if (qName.startsWith(ENTRY_TAG)
					&& tagStack.search(LIST_TAG) == PARENT_NODE) {
				entryDTO = new PwsEntryDTO();
			}
			if (qName.startsWith(GROUP_TAG)
					&& tagStack.search(ENTRY_TAG) == PARENT_NODE) {
				String treeName = attrs.getValue(TREE_ATTR);
				entryDTO.setGroup(treeName);
			}
			tagContent = new StringBuffer();
		}

		/**
		 * @see org.xml.sax.helpers.DefaultHandler#endElement(java.lang.String,
		 *      java.lang.String, java.lang.String)
		 */
		public void endElement(String uri, String localName, String qName)
				throws SAXException {
			
			if (qName.startsWith(ENTRY_TAG)) {
				entryList.add(entryDTO);
			}
			
			String prevTag = (String) tagStack.peek();
			
			String xmlText = tagContent.toString();

			if (prevTag.equalsIgnoreCase(GROUP_TAG)) {
				if (tagStack.search(ENTRY_TAG) == PARENT_NODE) {
					entryDTO.setGroup(xmlText);
				}
			} else if (prevTag.equalsIgnoreCase(TITLE_TAG)) {
				if (tagStack.search(ENTRY_TAG) == PARENT_NODE) {
					entryDTO.setTitle(xmlText);
				}
			} else if (prevTag.equalsIgnoreCase(USERNAME_TAG)) {
				if (tagStack.search(ENTRY_TAG) == PARENT_NODE) {
					entryDTO.setUsername(xmlText);
				}
			} else if (prevTag.equalsIgnoreCase(PASSWORD_TAG)) {
				if (tagStack.search(ENTRY_TAG) == PARENT_NODE) {
					entryDTO.setPassword(xmlText);
				}
			} else if (prevTag.equalsIgnoreCase(NOTES_TAG)) {
				if (tagStack.search(ENTRY_TAG) == PARENT_NODE) {
					entryDTO.setNotes(xmlText);
				}
			}
			
			
			tagStack.pop();
		}

		/**
		 * @see org.xml.sax.helpers.DefaultHandler#characters(char[], int, int)
		 */
		public void characters(char[] ch, int start, int length)
				throws SAXException {

			String xmlText = new String(ch).substring(start, start + length);
			tagContent.append(xmlText);

		}

	}

	/**
	 * @see au.gov.centrelink.itsecurity.tokenservice.saml.SubjectConfirmationDataParser#parse(java.lang.String)
	 */
	public PwsEntryDTO[] parse(String incomingXMLString) {

		PwsEntryDTO[] entries = null;

		try {
			XMLReader xmlReader = SAXParserFactory.newInstance().newSAXParser()
					.getXMLReader();

			XMLDataParserHandler xmlHandler = new XMLDataParserHandler();

			xmlReader.setContentHandler(xmlHandler);
			InputSource is = new InputSource(
					new StringReader(incomingXMLString));
			xmlReader.parse(is);
			entries = (PwsEntryDTO[]) xmlHandler.entryList
					.toArray(new PwsEntryDTO[0]);
		} catch (Exception e) {
			// Almost certainly related issues parsing the XML.
			throw new RuntimeException(e);
		}
		return entries;
	}

	public String convertToXML(PwsEntryDTO[] entries) {

		String outputString = "";

		DocumentBuilder db;
		try {
			db = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		} catch (ParserConfigurationException e) {
			// No XML Parsers installed, unlucky
			throw new RuntimeException(e);
		}
		Document doc = db.newDocument();

		Element list = doc.createElement(LIST_TAG);

		for (int i = 0; i < entries.length; i++) {

			PwsEntryDTO nextEntry = entries[i];
			Element el = doc.createElement(ENTRY_TAG);

			Element groupTag = doc.createElement(GROUP_TAG);
			groupTag.appendChild(doc.createTextNode(nextEntry.getGroup()));
			el.appendChild(groupTag);

			Element titleTag = doc.createElement(TITLE_TAG);
			titleTag.appendChild(doc.createTextNode(nextEntry.getTitle()));
			el.appendChild(titleTag);

			Element userTag = doc.createElement(USERNAME_TAG);
			userTag.appendChild(doc.createTextNode(nextEntry.getUsername()));
			el.appendChild(userTag);

			Element passwordTag = doc.createElement(PASSWORD_TAG);
			passwordTag
					.appendChild(doc.createTextNode(nextEntry.getPassword()));
			el.appendChild(passwordTag);
			
			Element notesTag = doc.createElement(NOTES_TAG);
			notesTag
					.appendChild(doc.createTextNode(nextEntry.getNotes()));
			el.appendChild(notesTag);

			list.appendChild(el);

		}

		doc.appendChild(list);

		try {
			TransformerFactory tFactory = TransformerFactory.newInstance();
			Transformer transformer = tFactory.newTransformer();
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");
			DOMSource source = new DOMSource(doc);
			StringWriter sr = new StringWriter();
			StreamResult result = new StreamResult(sr);
			transformer.transform(source, result);
			outputString = sr.toString();
		} catch (TransformerException te) {
			throw new RuntimeException(te);
		}

		return outputString;

	}

}
