import type { Metadata } from 'next'
import './globals.css'

export const metadata: Metadata = {
  title: 'HTH Bridge - Cross-Chain Token Bridge',
  description: 'Bridge HTH tokens across EVM chains using LayerZero',
}

export default function RootLayout({
  children,
}: {
  children: React.ReactNode
}) {
  return (
    <html lang="en">
      <body>{children}</body>
    </html>
  )
}
