/**
 * PAWSOMEBOND THEME
 * DARYX Tech Inc. Brand Colors
 * 
 * Use these colors throughout the React Native app.
 * See PDF guide for component-specific styling.
 */

export const COLORS = {
  // BACKGROUNDS
  background: '#0d1117',    // Main screen background
  card: '#161b22',          // Cards, headers, footers
  border: '#30363d',        // Borders, dividers
  
  // ACCENTS
  primary: '#00d4ff',       // Cyan - buttons, active states, highlights
  secondary: '#a855f7',     // Purple - secondary accents, steps chart
  lightCyan: '#7dd3fc',     // Hover states, subtle highlights
  
  // TEXT
  textPrimary: '#ffffff',   // Main titles, important text
  textSecondary: '#c9d1d9', // Body text, descriptions
  textMuted: '#8b949e',     // Captions, hints, inactive
  
  // STATUS
  success: '#22c55e',       // Connected, positive
  warning: '#f97316',       // Caution, limits
  error: '#ef4444',         // Disconnected, errors
};

// Component styles ready to use
export const STYLES = {
  // Screen wrapper
  screen: {
    flex: 1,
    backgroundColor: COLORS.background,
  },
  
  // Header bar
  header: {
    backgroundColor: COLORS.card,
    borderBottomWidth: 2,
    borderBottomColor: COLORS.primary,
    paddingVertical: 16,
    paddingHorizontal: 20,
  },
  
  // Cards
  card: {
    backgroundColor: COLORS.card,
    borderRadius: 12,
    borderWidth: 1,
    borderColor: COLORS.border,
    padding: 16,
    marginVertical: 8,
  },
  
  // Primary button (cyan)
  buttonPrimary: {
    backgroundColor: COLORS.primary,
    borderRadius: 8,
    paddingVertical: 12,
    paddingHorizontal: 24,
    alignItems: 'center',
  },
  buttonPrimaryText: {
    color: COLORS.background,
    fontWeight: 'bold',
    fontSize: 14,
  },
  
  // Secondary button (outline)
  buttonSecondary: {
    backgroundColor: 'transparent',
    borderRadius: 8,
    borderWidth: 1,
    borderColor: COLORS.border,
    paddingVertical: 12,
    paddingHorizontal: 24,
    alignItems: 'center',
  },
  buttonSecondaryText: {
    color: COLORS.textSecondary,
    fontSize: 14,
  },
  
  // Input fields
  input: {
    backgroundColor: COLORS.card,
    borderRadius: 8,
    borderWidth: 1,
    borderColor: COLORS.border,
    paddingVertical: 12,
    paddingHorizontal: 16,
    color: COLORS.textSecondary,
    fontSize: 14,
  },
  
  // Navigation bar
  navBar: {
    backgroundColor: COLORS.card,
    borderTopWidth: 1,
    borderTopColor: COLORS.border,
    flexDirection: 'row',
    justifyContent: 'space-around',
    paddingVertical: 12,
  },
  navActive: {
    color: COLORS.primary,
  },
  navInactive: {
    color: COLORS.textMuted,
  },
  
  // Chat bubbles
  chatUser: {
    backgroundColor: COLORS.primary,
    borderRadius: 16,
    padding: 12,
    maxWidth: '75%',
    alignSelf: 'flex-end',
  },
  chatUserText: {
    color: COLORS.background,
  },
  chatAI: {
    backgroundColor: COLORS.card,
    borderRadius: 16,
    borderWidth: 1,
    borderColor: COLORS.border,
    padding: 12,
    maxWidth: '75%',
    alignSelf: 'flex-start',
  },
  chatAIText: {
    color: COLORS.textSecondary,
  },
};

export default { COLORS, STYLES };
